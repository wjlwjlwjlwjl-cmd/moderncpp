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

![coroutine](C:\GitFiles\moderncpp\20\coroutine.PNG)

**一个协程生成器的开始、执行、销毁、异常处理逻辑示意**

## 2.2 Promise

Promise是协程的心脏，决定了协程的操作句柄、异常处理、返回情况、变量存储，以及在开始、终止、生成数值是的挂起情况。

### 2.2.1 get_return_object

C++20的协程操作句柄 `coroutine_handle`，是通过 `Promise` 初始化的，一个 `Promise` 类型，需要在内部实现例子中的各种方法供给编译器自己去调用，而 `get_return_object`，产生的就是自定义的包装Promise给外界使用接口的类型（如上面例子中的Generator）

### 2.2.2 initial_suspend、final_suspend

决定协程函数在开始调用（执行第一个语句前）和结束（ `return_void` 或者 `return_value` ）之后是否需要挂起。如果挂起的话，只有外界通过对应的协程操作句柄 `resume` ，才能够继续往下执行。注意 `final_suspend` 需要实现为 `noexcept`

### 2.2.3 yield_value

在协程函数调用 `co_yield`时，用来完成对 `Promise` 内数据的操作逻辑。同样 `yield_value` 可以决定后续是否挂起

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

### 2.2.7 operator coroutine_handle<>()

该转换函数将值转换为具有相同底层 `address` 地址的协程句柄，可以在 `suspend_await`中作为形参接受各协程的句柄

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

## 2.3 Awaitable对象

1. 在C++20中，任何出现在 `co_await` 左侧的对象都是 `Awaitble` 对象。它决定了以下三种情况

* 是否需要暂停当前协程
* 在暂停之前，需要完成那些处理逻辑
* 在恢复之后，如何获取异步操作的结果

2. 为了完成以上三种情况，必须在自己实现的 `Awaitable`  对象中完成 `await_ready` `await_suspend` `await_resume`

### 2.3.1 bool await_ready() const

决定请求挂起后，条件是否就绪，是否真的需要挂起。如果真的需要挂起，返回false，否则true

### 2.3.2 type await_suspend(std::coroutine_handle<>) const

在调用完了 `await_ready` ，确定要挂起后，实际挂起之前，会调用 `await_suspend`，完成挂起之前的操作：获取某个协程的句柄，将它交给另一个异步操作在事件就绪之后再将被挂起的协程恢复

**返回值决定了 `await_suspend` 之后的操作**

* 返回 void，则控制权转移到协程的调用者，协程暂停。这也是最常见的情况
* 返回 true，代表已经挂起到后台，后面也可能在另一个线程中恢复；返回 false，代表在挂起前的最后等待事件就绪，协程不再挂起，继续往下执行
* 返回 `coroutine_handle<>` 代表返回另一个协程句柄，不像前面两种方式一样需要一个“调度器”角色的第三方来判断是否需要往下执行，控制权不再返回到调用者，而是通过返回的协程句柄到相应协程继续执行。这就是协程的对称转移，是实现无栈链式协程调用的关键

### 2.3.3 type await_resume() const

在协程挂起恢复后或者 `await_ready` 返回 true 后，会调用 `await_resume`，来产生 `co_await` 的返回值，在不抛出异常时，建议和前面个两个一样，实现为 `noexcept`

### 2.3.4 operator co_await()

使得我们自定义的类型，成为能够被 co_await 调用的 Awaitable 对象

>**std::suspend_always** **std::suspend_never**，是库中为我们提供的两个 Awaitable 对象，第一个表示立即挂起，第二个表示不挂起继续往下执行协程
>
>两者常用于 `initial_suspend` `final_suspend` `yield_value` 等的返回值来确定是否需要挂起

# 3. 模块

>在有模块以前，C++对于所有包含的头文件，在编译时都会进行展开并全部重新进行编译，哪怕只有一个文件的一个函数做出了微小的更改。虽然也可以通过对于类只传指针来降低类修改后整体编译的时间，但是整体上依然很慢。

C++20引入了模块，允许模块中的内容实现 **编译一次，多次调用** 的效果。同时，相比C++一直使用的头文件机制，模块有以下优点：

1. 缩短编译时间。对于多个文件同时使用的模块，不需要像以前一样每个文件编译时都编译自己的一遍。模块的实现文件（或者自己声明并实现的文件）会作为翻译单元单独参与编译
2. 隔离性。除非是导出的内容，模块中的所有实现都不可见，全局函数、变量等只在模块中有效
3. 无宏泄露。模块中定义的宏不会延伸到导入模块的文件中
4. 更清晰的语义，明确区分接口和实现。一般建议在 `.ixx` 文件中书写声明（模板除外），在 `.cpp` 中书写定义，但是就像头文件中可以直接实现函数体，在 `.ixx` 中也是可以写定义的
5. 更快的增量编译。一个模块发生修改后，只需要重新编译这个模块的内容

> 模板除外的原因是：在调用模板的单元中，因为没有定义（模板的声明在头文件），所以在编译这个单元时无法实例化，会等到链接时在试图寻找；但是当编译定义该模板的单元时，返现并没有调用它，所以不会为它生成代码。
>
> 这样到最后也没有找到该模板的实例化代码，所以链接报错

## 3.1 模块的基本语法规则

### 3.1.1 模块的声明、导出、与导入

在 `.ixx` 文件中，使用 `export module ModelName` 的方式声明接下来模块的实现。

在需要导出的内容之前，加上 `export` 关键字，来导出相应内容，如：

```cpp
export module Func;
export void func(){};  	  //导出函数
export{			 	  	 //同时导出多个函数
    void func1(){};
    void func2(){};
}
export namespace A{	   	  //导出命名空间
    void funcA(){};
}
export template<class T>  //导出模板
T add(T x, T y){return x + y;}
export class B{			 //导出结构体
public:
    void funcB(){};
};
```

只有模块中使用 `export` 导出的内容，在 `import` 的文件中才是可见的，其他的函数、变量、包含的头文件（在全局模块片段中），都是不可见的，只在模块内部可访问

在模块的实现文件中，通过 `export ModelName` 来表示自己实现的是哪一个模块的内容

在需要使用模块的文件中，使用 `import ModelName` 来导入模块

### 3.1.2 全局模块片段

如果需要在模块中使用其他头文件中的内容，而这些文件又没有模块化，就需要使用**全局模块片段**了。（如果直接包含的话，会报警告）

全局模块片段使用 `module` 关键字开启，到模块的声明结束，例如

```cpp
module;
#include <iostream>
#define BUFFER_SIZE 1024;

export module Func;
void func(){
	std::cout << BUFFER_SIZE << std::endl;
}
```

全局模块片段中只能使用预编译指令。如果使用非预编译指令会直接编译报错

## 3.2 分区

当模块达到一定规模之后，就需要引入分区。从某个方面可以理解为”模块的模块“，使得模块也能够各个分区分离编译。

模块分区具有以下特点：

* 分区是模块的内部实现细节，彼此不导入的话，也无法使用彼此export的内容

* 外部⽤⼾⽆法直接导⼊分区，只可以导入主模块接口 `export import` 的其他分区

* 分区之间可以相互引⽤，哪怕循环也可以

* 主模块接⼝负责导出分区内容

### 3.2.1 语法结构

1. 分区声明

```cpp
export module ModelName: PartitionName;
```

2. 分区实现

```cpp
module ModelName: PartitionName;
```

3. 分区导入

```cpp
import :PartitionName;
```

只有导入了其他分区，才能够使用其他分区的内容。

对于主模块接口而言，使用 `import` 导入的分区只有自己可以使用，如果 `export`导出的接口中使用了这些内容，也是无法使用的。除非使用 `export import PartitionName`，才可以让外界也可以使用这些分区内容

4. 分区导出

```cpp
export import PartitionName;
```

这就是在主模块中实现的，向外界提供的分区

### 3.2.2 文件结构

* 一般建议模块声明文件放在 `.ixx` 或者 `.cppm` 文件中，实现放在 `.cpp` 中
* 只可从模块接口单元中导出声明，即 `export` 只能放在 `.ixx` 这种声明文件中
