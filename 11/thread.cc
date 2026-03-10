#include <iostream>
#include <mutex>
#include <thread>

std::mutex foo, bar;
int i = 0;
std::once_flag flag;
void set_winner(const std::string &s) {
    std::cout << "winner is " << s << std::endl;
}

void task1() {
    std::unique_lock<std::mutex> lk(foo, std::try_to_lock);
    std::unique_lock<std::mutex> lk2(bar, std::try_to_lock);
    if (lk) {
        std::cout << "task1 get the lock" << std::endl;
    }
}

void task2() {
    int ret = std::try_lock(foo, bar);
    if (ret == -1) {
        std::cout << "get all lock" << std::endl;
    }
    if (ret == 1) {
        std::cout << "haven't get bar" << std::endl;
    }
    if (ret == 0) {
        std::cout << "haven't get foo" << std::endl;
    }
}

void race(const std::string &name) { std::call_once(flag, set_winner, name); }

int main() {
    std::thread t1(task1);
    std::thread t2(task2);
    std::thread t3(race, "t3");
    std::thread t4(race, "t4");

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    return 0;
}
