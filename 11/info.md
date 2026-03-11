# thread库
1. `thread` 对象创建的方式常用有两种，一种是给予可调用对象和参数，另一种是通过移动构造，线程对象不允许拷贝构造<br>
<strong>通过移动构造</strong>
```cpp
std::vector<std::thread> threads(2);
for (int i = 0; i < 2; i++) {
    threads[i] = std::thread(Print, 1, 2);
}
```
<strong>通过给予参数</strong>
```cpp
void Print(int a, int b)
{
    std::cout << a + b;
}
std::thread t1(Print, 1, 10);
```
线程join使用`join()`;
# lock_guard和unique_lock
## lock_guard
1. 更加简单，不能手动`lock`、`unlock`，不支持拷贝，不能移动构造，可以通过用户括号构造局部域的方式来限制加锁范围
```cpp
void counter() {
    for (int i = 0; i < 100000; i++) {
        std::lock_guard lock(foo);
        std::cout << i << std::endl;
    }
}
```
## unique_guard
1. 相比`lock_guard`，支持更加丰富的操作，允许延迟加锁或者构造时不上锁等
2. 选项有
* `adopt_lock`，不再上锁，可以应用在防止死锁的情况下
```cpp
void task1() {
    //这样只有在foo和bar两个锁都获取时才会往下处理
    std::lock(foo, bar);
    std::unique_lock<std::mutex> lock1(foo, std::adopt_lock);
    std::unique_lock<std::mutex> lock2(bar, std::adopt_lock);
    std::cout << "foo and bar get: " << std::this_thread::get_id() << std::endl;
}
```
* `defer_lock`，推迟上锁，默认在构造时上锁，但如果指定此选项就构造时上锁
```cpp
void task2() {
    std::unique_lock<std::mutex> lock1(foo, std::defer_lock);
    std::unique_lock<std::mutex> lock2(bar, std::defer_lock);
    std::lock(lock1, lock2);
    std::cout << "foo and bar get: " << std::this_thread::get_id() << std::endl;
}
```
上面的例子就说明了`unique_lock`支持手动上锁，通过调用lock try_lock unlock等接口
* `try_to_lock`，尝试上锁，没有上锁也不会阻塞等待
```cpp
void task3() {
    std::unique_lock<std::mutex> lk(foo, std::try_to_lock);
    if (lk) {
        std::cout << "get foo" << std::endl;
    } else if (lk.owns_lock()) {
        std::cout << "haven't get foo" << std::endl;
    }
}
```
在上面的例子中，通过两种方式判断是否完成上锁，一种是`unique_lock`重载了`operator bool`，另一种是`owns_lock`;
## lock和try_lock解决死锁问题
* 先来看下面的代码
```cpp
std::mutex foo, bar;
void task1()
{
    foo.lock();
    bar.lock();
    foo.unlock(); bar.unlock();
}
void task2()
{
    bar.lock();
    foo.lock();
    foo.unlock(); bar.unlock();
}
```
在上面的代码中，就可能会出现死锁，原因是task1先锁住foo，task2在锁住bar，两者接下来都阻塞在获取对方已经锁上的
### lock
可以同时等待多把锁的锁住，如果没能够一次等待完成所有的锁，就释放已经获取的锁，然后阻塞等待，直到获取所有等待的锁
### try_lock
同样的等待多把锁，但是会返回没有成功等待到的锁的序号（和数组一样，根据调用时指定锁的顺序，从零开始），如果获取了所有锁，就返回-1（表示没有等待失败的锁）
```cpp
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
```
### call_once
这个函数只会被执行一次（有`once_flag`区分），回调函数和参数也要一并指定，例如以下判断哪个线程首先完成工作的程序
```cpp
std::once_flag flag;
void set_winner(const std::string &s) {
    std::cout << "winner is " << s << std::endl;
}
void race(const std::string &name) { std::call_once(flag, set_winner, name); }
```
# atomic
1. `atomic`是C++通过原子指令来让一段代码能够在不使用锁的情况下就保证线程安全的方法，具体操作是基于cpu的原子指令来实现的，适合临界区不太大的情况，这样就不需要像互斥锁一样让没有获取锁的线程阻塞等待，从cpu上切换下去（切换上下文是由开销的），例如以下代码就完成了模仿`atomic`的`operator++`;
```cpp
void Add(std::atomic<int> &cnt) {
    int old = cnt.load();
    //通过全局函数的方式调用
    while (!atomic_compare_exchange_weak(&cnt, &old, old + 1))
        ;
    //通过成员函数的方式
    while(cnt.compare_exchange_weak(old, old + 1));
}
```
以下代码完成了多线程下链表节点插入的安全性维护
```cpp
typedef struct Node {
    int val;
    struct Node *next;
    Node(int val) : val(val), next(nullptr) {}
} Node;
std::atomic<Node *> list_head;

void push_back(int val) {
    Node *oldNode = list_head.load();
    Node *newNode = new Node(val);
    while (!list_head.compare_exchange_weak(oldNode, newNode)) {
        newNode->next = oldNode;
    }
}
```
`atomic_compare_exchange_weak`是通过将`cnt`与`old`比较，如果两者相等，就将cnt的值设置为`old + 1`，并返回`true`，否则就将`cnt`设置为`old`的值，并返回`false`;<br>
`atomic`对于类型的要求，简单来说就是“简单、固定大小、浅拷贝即可”的类型才能够作为atomic作用的类型
2. 关于`compare_exchange_weak`和`compare_exchange_strong`的区别，在于前者可能会出现假失败的情况（关心的值并没有发生改变，而是因为cpu线程切换、其他线程访问操作内存等原因失败），但是后者通过内部再次嵌套一层循环的方式判断，直到真的出现关心的值不相等的时候才会返回，但是在某些平台上效率相对低一些
## 通过`atomic`或者`atomic_flag`实现自旋锁
1. 自旋锁就是没有获取到锁的线程不再阻塞等待，也就节省了切换上下文的开销，而是类似于轮询的方式不断试图获取到锁，适合临界区不太大的情况（C++标准库中没有对自旋锁进行实现），但是也有cpu资源的浪费，所以要合理选用
2. 可以通过`atomic`实现，其中`exchange`接口会返回设置以前的值
```cpp
class SpinLock {
public:
    SpinLock() : _flag(false) {}

    void lock() {
        while (_flag.exchange(true))
            ;
    }

    void unlock() { _flag.store(false); }

private:
    std::atomic<bool> _flag;
};
```
3. 通过`atomic_flag`实现的自旋锁更加清量，不同于使用`atomic`，主要提供`test_and_set()`和`clear()`两种方式来完成值的设置，同时初始化必须使用`ATOMIC_FALG_INIT`进行，默认初始化为`false`
```cpp
class SpinLock2 {
public:
    SpinLock2() : flag(ATOMIC_FLAG_INIT) {}

    void lock() {
        while (flag.test_and_set())
            ;
    }

    void unlock() { flag.clear(); }

private:
    std::atomic_flag flag;
};
```

# condition_variable条件变量
1. `condition_variable`是C++标准库对各平台条件变量相关接口的封装，其实使用思路和Linux pthread库中的几乎一摸一样。
2. 传给`condition_variable`的锁必须是通过`unique_lock`管理的锁
3. 例如以下面试经典题，使用两个线程交替打印奇数和偶数
```cpp
bool flag = false;
std::thread t1([&]() {
    int i = 0;
    while (i < 100) {
        std::unique_lock<std::mutex> lk(mtx);
        if (flag)
            cv.wait(lk);
        std::cout << i << " ";
        i += 2;
        flag = true;
        cv.notify_one();
    }
});

std::thread t2([&]() {
    int j = 1;
    while (j < 100) {
        std::unique_lock<std::mutex> lk(mtx);
        if (!flag)
            cv.wait(lk);
        std::cout << j << " ";
        j += 2;
        flag = false;
        cv.notify_one();
    }
});
t1.join();
t2.join();
```
# future和async
1. `future`用于获取异步操作的结果，允许在一个线程中启动任务，在另一个线程中等待任务的完成并接受返回值，支持检查任务状态
2. `async`是cpp的异步任务接口，提供了多种启动策略，如`deffered`, `async`，其中deffered为延迟执行，当调用get时开始执行，async为立刻执行;对线程的各种实现都进行了封装，对异常自动捕获并传递，线程结束通过`future`带出（注意需要显式接收，否则线程会阻塞在临时对象的析构函数中），主线程的接收可以通过线程返回的`future`调用`wait`,`wait_for`,`wait_until`获取结束状态，有`future_status`中枚举的`deffered` `timeout` `ready`描述状态，也可以通过`future`调用`get`阻塞等待并接收任务的返回值。需要注意的是`get`只能调用一次，会转移线程的所有权，调用`valid`会返回`false`并且再次调用会出现未定义行为，当异步操作抛出异常时，会通过`get`将异常传递到主线程
3. 使用`async`和直接使用`thread`对象的区别
* 前者封装的更好自动管理线程，不需要像后者一样手动创建，也意味着灵活性低
* 前者异常自动捕获传递，后者需要手动处理
* 前者通过`future`自动传递结果，后者需要手动处理
4. `wait` `wait_for` `wait_until` `get`的区别
* `wait`只关心异步任务是否完成，不在乎状态与返回值
* `wait_for` `wait_until`通过设置时间段、时间点来为任务设置一个“期限”，当时间耗尽或者任务完成结束阻塞，同时会返回`future_status`来表示异步调用状态
* `get`会一直阻塞，并返回调用任务的返回值，同时如果异步调用期间出现异常也会在这里传递给主线程
```cpp
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
```
