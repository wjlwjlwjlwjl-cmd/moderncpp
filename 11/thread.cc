#include <future>
#include <iostream>
#include <thread>

int task(const std::string &name) {
    std::cout << name << " start" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << std::this_thread::get_id() << " " << name << std::endl;
    return 1;
}

int main() {
    auto fut1 = std::async(std::launch::async, task, "async task");
    auto fut2 = std::async(std::launch::deferred, task, "deffered task");
    std::cout << "waiting result" << std::endl;

    auto ret = fut1.wait_for(std::chrono::seconds(1));
    if (ret == std::future_status::timeout) {
        std::cout << "task async timeout" << std::endl;
    }
    auto res = fut2.get();
    std::cout << std::boolalpha << fut2.valid() << std ::endl;
    return 0;
}
