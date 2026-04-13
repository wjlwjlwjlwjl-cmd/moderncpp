# 1. 概念与约束

## 1.1 概念的定义方式

通过 concept 声明一个概念，主要语法格式如下：

```cpp
template <template-parameter-name>
concept concept-name = constraint-expression;
```

其中，template-parameter-name 是模板参数列表；concept-name是概念名称，constraint-expression是能够求值为 bool 的常量表达式。

constraint-expression 可以通过以下方式给定

1. 标准库的类型萃取

```cpp
template <class T>
concept C1 = std::is_integral_v<T>;
```

2. 常量表达式

```cpp
template <class T>
concept C2 = sizeof(T) < sizeof(long);
```

3. requires 约束

```cpp
template <class T>
concept C3 = requires(T i){
    i++; --i;
};
```

在上面 requires 中并不会执行实际操作，而是判断是否能够完成编译。在上面的例子中，即：T类型是否支持后置++ 和前置-- 的操作

## 2.2 requires的四种方式

### 2.2.1 简单约束

```cpp
template <class T>
concept C4 = requires(T a, T b){
    a += b;
};
```

### 2.2.2 类型约束

```cpp
template <class T>
concept C5 = requires{
    typename T::iterator;
    typename T::value_type;
};
```

**关于 typename**：当试图去除类中的成员类型时，需要加上 typename 来表示不是要实例化某个类型

### 2.2.3 复合约束

复合约束的基本语法 { expression } [noexcept] -> 类型约束

```cpp
template <class T>
concept C6 = requires(T a, T b){
    {a + b} -> convertible_to<T>; //a + b的结果能够转换为T
    {a += b} noexcept -> same_as<T&>; //a += b不会抛异常，并且结果是T的引用
};
```

### 2.2.4 嵌套约束

```cpp
template <class T>
concept C7 = requires{
    requires std::is_class_v<T>;
    requires sizeof(T) < sizeof(long);
    requires requires(T t){
        t++;
    }; //有点类似与创建临时变量的方式创建一个临时的要求
    C5;
};
```

嵌套约束中嵌套的可以是其他约束（简单要求、复合约束......），也可以是 requires + 常量表达式，或者直接通过概念（其实也可以认为等价于前者）

## 2.3 约束的使用

1. 直接在模板参数列表中使用

```cpp
template <C1 T>
void f1(T t){}
```

2. requires 子句或者尾置 requires 子句

```cpp
template <C2 T>
requires C1<T> || C2<T>
void f2(T t){}

template <C3 T>
void f3(T t) requires C1<T> || C2<T> {}
```

3. 临时约束

```cpp
template <C4 T>
requires requires(T x){sizeof(T) < sizeof(long);}
void f4(T t){}
```

4. auto占位符，帮助省略模板参数列表

```cpp
void f4(C5 auto t){}
```

## 2.4 约束的种类

### 2.4.1 原子约束

即一个不能在拆分的约束，比如一个概念名，如 std::Integer\<T\>；一个 requires 表达式；一个常量布尔表达式等

### 2.4.2 合取和析取

可以通过`||`或者`&&`的方式完成将两个约束通过逻辑判断的方式合并在一起，也存在短路的行为，例如前面的 Integer\<T\> || Float\<T\>，就是析取

### 2.4.3 可变参数包的requires表达式

这两个在官方文档上是要到C++26实现，但是 MSVC 已经支持了这种方式，可以对可变参数包批量进行我们约束的判断 

```cpp
template <class...Arg>
requires Integer<Arg...>
void func(Arg...args) {
	std::cout << sizeof...(args) << std::endl;
}
```

### 2.4.4 约束的偏序规则

在一个模板有被多个约束修饰的重载时，编译器会从所有重载中选择一个约束最严的调用。比如A约束被C约束包含，这就意味着满足了C就一定满足了A，但是反过来不一定。所以会优先调用C约束的重载

```cpp
template <class T>
concept A = std::is_copy_constructible_v<T>;

template <class T>
concept B = std::is_move_constructible_v<T>;

template <class T>
concept C = B<T> && A<T>;

template <A T>
void func(T t) {
	std::cout << "A->func" << std::endl;
}
template <C T>
void func(T t) {
	std::cout << "C->func" << std::endl;
}
```

比如在上面的例子中，调用 func(1.1) A、C两个约束都成立，但是会打印 C->func，因为C更严格。

# 2. 协程

## 2.1 What's 协程

如果将线程比作一个工人的话，那么多线程就是让多个工人各自去干各自的活。但是当一位工人的工作暂时不能继续往下执行（比如等待IO事件），这位工人也就干等着，线程就被阻塞了，我们的并行计算能力就没有得到最大效率的发挥。

协程就是让我们实现一个用户级的“多线程环境”。多个函数中，每当一个函数需要等待时，就进入阻塞状态，返回对它调用的函数继续执行，当外界让他继续（resume）时，再中断刚才的执行，回到挂起前的地方继续执行。

如果继续用之前的比喻继续解释的话，就是：一个工人手头的工作需要等待时，就让工人去做其他的事情，而不是干等着。

除了对等待时间的利用更高之外，这种用户级别的“多线程”还不需要内核态 - 用户态的切换、上下文的处理等。

C++的协程是无栈协程，这意味着一个流程中，当去继续执行被挂起的函数时，就要从当前函数中暂停运行。在这个无栈协程的“栈”中，只需要保存局部状态（如恢复点）。

## 2.2 Promise

Promise是协程的心脏，决定了协程的操作句柄、异常处理、返回情况、变量存储，以及在开始、终止、生成数值是的挂起情况。

### 2.2.1 get_return_object

C++20的协程操作句柄 `coroutine_handle`，是通过 `Promise` 初始化的，一个 `Promise` 类型，需要在内部实现例子中的各种方法供给编译器自己去调用，而 `get_return_object`，产生的就是调用协程函数第一步返回的操作句柄

### 2.2.2 initial_suspend、final_suspend

决定协程函数在开始调用（执行第一个语句前）和结束（ `return_void` 或者 `return_value` ）之后是否需要挂起。如果挂起的话，只有外界通过对应的协程操作句柄 `resume` ，才能够继续往下执行。注意 `final_suspend` 需要实现为 `noexcept`

### 2.2.3 yeild_value

在协程函数调用 `co_yeild`时，用来完成对 `Promise` 内数据的操作逻辑。同样 `yeild_value` 可以决定后续是否挂起

### 2.2.4 return_void、return_value

用来确定协程函数的返回情况。`return_void` 和 `return_value` 只能实现一个，编译器会根据 `co_return` 的调用情况自主去调用相应实现

### 2.2.5 unhandled_exception

在协程函数中，如果抛出了异常并且没有 `catch` 的话，就会自动到 `unhandled_exception` 去执行异常处理逻辑，可以直接终止流程，也可以获取 `current_exception`，再 `rethrow_exception`

```cpp
struct promise_type {
	int current_value; 
	std::exception_ptr ex_ptr;
	int ret;
	Generator get_return_object() {
		return Generator{
		std::coroutine_handle<promise_type>::from_promise(*this) };
	}
	std::suspend_always initial_suspend() { return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }
	std::suspend_always yield_value(int value) {
		current_value = value;
		return {};
	}
	//void return_void() {}
	void return_value(int val) {
		ret = val;
	}
	void unhandled_exception() {  
		ex_ptr = std::current_exception();
	}
};
```

## 2.2 协程句柄

### 2.2.1 What's 协程句柄

协程句柄允许外界操作一个协程帧（类似于栈，不过是一个协程的所有函数所共享的一个“栈”）。在析构时，可以通过类似RAII的方式销毁协程帧（如果 `final_suspend` 挂起的话）

### 2.2.2 from_promise()、promise()

`from_promise()`，通过 `Promise` ，去获取一个指向 `Promise` 对象的操作句柄；`promise()`，方便我们通过句柄去获取 `Promise` 对象及其内部值等

### 2.2.3 resume()

让处于挂起状态的协程从上次挂起的位置重新往下执行。

**对于未挂起状态的 `resume`，是未定义行为**

### 2.2.4 done()

检查一个协程是否已经执行完成，即是否已经调用 `co_return` 或者函数体是否已经执行完成

### 2.2.5 operator bool()

判断一个句柄是否是有效的句柄（协程句柄对于Promise对象的指向使用的是 `void*` 指针，所以可以用 `nullptr` 初始化

### 2.2.6 destroy()

销毁协程帧，不过对于 `final_suspend` 选择 `std::suspend_never` 的来说，不需要手动实现，并且二次销毁还是未定义行为

```c++
struct Generator {
	struct promise_type {
		//... 前面的实现省略
	};
	std::coroutine_handle<promise_type> handle;
	explicit Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
	~Generator() {
		if (handle) handle.destroy();
	}
	int value() const {
		return handle.promise().current_value;
	}
	bool next() {
		if (!handle.done()) {
			handle.resume();
		}
		if (handle.promise().ex_ptr) {
			std::rethrow_exception(handle.promise().ex_ptr);
		}
		return !handle.done();
	}
};
```



![coroutine](C:\GitFiles\moderncpp\20\coroutine.PNG)
