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
这个函数只会被执行一次（有once_flag区分），回调函数和参数也要一并指定，例如以下判断哪个线程首先完成工作的程序
```cpp
std::once_flag flag;
void set_winner(const std::string &s) {
    std::cout << "winner is " << s << std::endl;
}
void race(const std::string &name) { std::call_once(flag, set_winner, name); }
```
