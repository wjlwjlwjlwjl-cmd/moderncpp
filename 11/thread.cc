#include <cmath>
#include <future>
#include <iostream>
#include <queue>
#include <thread>

bool isPrimer(int n) {
    if (n == 2)
        return true;
    if (n <= 1 || n % 2 == 0)
        return false;
    for (int i = 3; i <= std::sqrt(n); i += 2) {
        if (n % i == 0)
            return false;
    }
    return true;
}

int getPrimer(int start, int end) {
    int ret = 0;
    for (int i = start; i < end; i++) {
        if (isPrimer(i)) {
            ret += 1;
        }
    }
    return ret;
}

int main() {
    std::vector<std::future<int>> futures;
    for (int i = 0; i < 4; i++) {
        futures.emplace_back(std::async(std::launch::async, getPrimer,
                                        i * 10000, (i + 1) * 10000 + 1));
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
    int total = 0;
    for (auto &e : futures) {
        total += e.get();
    }
    std::cout << "[0, 40000] 中素数个数为: " << total << std::endl;
    return 0;
}
