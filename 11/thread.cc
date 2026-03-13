#include <future>
#include <iostream>
#include <queue>
#include <thread>

void runner(std::queue<std::packaged_task<int()>> &taskq, std::mutex &mtx,
            std::condition_variable &cv) {
    while (1) {
        std::packaged_task<int()> task;
        {
            std::unique_lock<std::mutex> ul(mtx);
            cv.wait(ul, [&taskq]() { return !taskq.empty(); });
            task = std::move(taskq.front());
            taskq.pop();
        }
        if (!task.valid()) {
            break;
        }
        task();
        {
            std::lock_guard lg(mtx);
            std::cout << std::this_thread::get_id() << " doing the work..."
                      << std::endl;
        }
    }
}

int main() {
    std::queue<std::packaged_task<int()>> taskq;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread rth(runner, std::ref(taskq), std::ref(mtx), std::ref(cv));
    for (int i = 0; i < 5; i++) {
        std::packaged_task<int()> task([i]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return i * i;
        });
        auto fut = task.get_future();
        {
            std::lock_guard lk(mtx);
            taskq.push(std::move(
                task)); // push有两种方式，一种是传左值引用，走拷贝构造；另一种是穿右值。这里如果使用ref的话，编译报错
        }
        cv.notify_one();
        std::cout << "task " << i << " get result: " << fut.get() << std::endl;
        if (i == 4) {
            std::lock_guard lk(mtx);
            taskq.emplace();
            cv.notify_one();
        }
    }
    rth.join();
    return 0;
}
