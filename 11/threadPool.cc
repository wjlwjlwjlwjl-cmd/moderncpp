#include <cmath>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
    ThreadPool(size_t tnum) : stop(false) {
        for (int i = 0; i < tnum; i++) {
            workers.emplace_back([=]() {
                while (1) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> ul(mtx);
                        cv.wait(ul, [=]() { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) {
                            std::cout << std::this_thread::get_id()
                                      << " exit..." << std::endl;
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template <class F, class... Arg>
    auto enqueue(F &&f, Arg &&...args)
        -> std::future<typename std::result_of<F(Arg...)>::type> {
        using return_type = typename std::result_of<F(Arg...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Arg>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::lock_guard lg(mtx);
            tasks.push([task]() { (*task)(); });
        }
        cv.notify_one();
        return res;
    }

    ~ThreadPool() {
        {
            std::lock_guard lg(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto &th : workers) {
            th.join();
        }
    }

private:
    std::condition_variable cv;
    std::mutex mtx;

    // 这里使用包装器的原因是为了允许一个线程池的任务队列中有多种参数的任务，packaged_list直接使用会导致任务参数固定
    std::queue<std::function<void()>> tasks;

    std::vector<std::thread> workers;
    bool stop;
};

bool isPrimer(int val) {
    if (val == 1)
        return false;
    if (val == 2)
        return true;
    if (val % 2 == 0)
        return false;
    for (int i = 3; i <= std::sqrt(val); i += 2) {
        if (val % i == 0)
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
    ThreadPool tp(4);
    auto fut1 = tp.enqueue(getPrimer, 0, 10000);
    auto fut2 = tp.enqueue(getPrimer, 10001, 20000);
    auto fut3 = tp.enqueue(getPrimer, 20001, 30000);
    auto fut4 = tp.enqueue(getPrimer, 30001, 40000);
    int ret = fut1.get() + fut2.get() + fut3.get() + fut4.get();
    std::cout << "[0, 40000]中素数个数为：" << ret << std::endl;

    return 0;
}
