### 结构化绑定

1. 结构化绑定允许从结构体、数组、元组等结构中快速简单的绑定多个对象，通过形如`auto [elem1, elem2, ...] = taget_obj`
2. 结构化绑定的参数个数必须和被绑定对象中的对象个数相同
3. 可以绑定类对象或者结构体对象，但是被绑定成员必须是公有成员，私有成员在成员函数内部绑定是可以的（类内部不受限定符约束）
4. 可以通过`auto&` `auto&&`的方式去初始化引用对象，注意后者是万能引用
5. 结构化绑定生成变量的声明周期和被绑定对象的相同
6. 不允许嵌套绑定

```cpp
std::tuple t1{1, 1.1, "hello world"};
auto& [x, y, z] = t1;

std::unordered_map<int, string> hash;
for(auto& [x, y]: hash){
    std::cout << "key: " << x << " val: " << y << std::endl;
}
```

### inline 变量

1. 在头文件中直接定义定义函数或者变量时，如果是非static非const变量的话，那么在链接时就会连接到重复定义的变量而报错（包含了这个头文件的各个编译单元人手一份自然就重复了，没有`inline`之前，需要如下解决

```cpp
//header.h
extern int a;
//test.cpp
int a = 0;
//main.cpp
std::cout << a << std::endl;
```

这是第一种方法，通过`extern`声明外部变量来解决，但是有两点缺点：

* 麻烦
* 当不同编译单元分别作出定义、修改时，会类似写时拷贝一样开出来两个变量，名字相同但是地址不同

```cpp
//header.h
const int a = 10;
static int b = 10;
//test.cpp main.cpp 直接使用
```

这是第二种方法，使用const、static修饰，但是这也是会让各个编译单元拥有名字、值相同但是地址不同的变量

2. C++17引入了`inline`关键字修饰变量的新特性，对于头文件中使用`inline`的变量，编译器会从多个定义中保留一个

```cpp
//header.h
inline int a = 10;
//testcpp main.cpp 直接使用
```

### 在switch/if中初始化变量

```cpp
vector<int> v{1, 2, 3};
if(auto it = find(v.begin(), v.end(), 1); it != v.end()){
    std::cout << "have 1" << std::endl;
}
```

在`switch`语句中完全相似

### 强制省略删除（返回值优化标准化）

1. 首先指明一下两种分类：NRVO(Named Return Value Optimization) URVO(Unnamed Return Value Optimization)
2. C++17强制规定，无名返回值必须优化，NRVO由编译器自己决定

```cpp
class Noisy {
public:
	Noisy() { std::cout << "construction..." << std::endl; }
	Noisy(const Noisy& n) { std::cout << "copy construction..." << std::endl; }
	Noisy(Noisy&& n) { std::cout << "move construction..." << std::endl; }
	~Noisy() { std::cout << "deconstruction..." << std::endl; }
};

Noisy make_nrvo() {
	//按理来说这里是有以下流程：构造、拷贝构造、拷贝构造(或者这两个拷贝构造使用移动构造)、构造。这里是NRVO，C++标准没有规定，但编译器通常会合三为一
	//output: construction... deconstruction...
	Noisy v = Noisy();
	return v; 
}

Noisy make_urvo() {
	//按理来说这里应该是构造、拷贝构造、拷贝构造，这里直接实现为一次构造
    //output: construction... deconstruction...
	return Noisy();
}

int main() {
	auto ret = make_urvo();
	return 0;
}

```

3. 虽然市面上主流的编译器(gcc/g++ vs clang)都会进行自己的优化，但是这毕竟是编译器级别而不是语言级别的，所以仍然需要有相应的构造函数支持

```cpp
Noisy(const Noisy& n) = delete;
Noisy(Noisy&& n) = delete;
```

这样处理之后，上面例子中的`make_nrvo`就会报错，虽然编译器会进行优化，但这并不是语言规定的，是可选的；而`make_urvo`不受影响，因为C++17标准强制规定这里直接省略为一次构造

### if constexpr

1. 前面其实已经提到过C++17支持的`if constexpr`，不同于`if`，其具有以下特点

* 编译器判断
* 条件是常量表达式
* 不符合条件的部分删除

```cpp
//在模板元编程中的使用——解包
template <class T, class...Arg>
void PrintArgs(T elem, Arg&&...args){
    if constexpr(sizeof...(args) > 0){
        std::cout << elem << " ";
        PrintArgs(args...);
    }
    else{
        //在编译时就自动生成了递归解包的返回特化
        std::cout << std::endl;
    }
}

//类型分发
template <class T>
void typeHandle(T&& type){
    if constexpr(std::is_same_v<T, int>){
        std::cout << "T is int" << std::endl;
    }
    else if constexpr(std::is_same_v<T, std::string>){
        std::cout << "T is string" << std::endl;
    }
    else{
        std::cout << "Invalid type" << std::endl;
    }
}
```

### 折叠表达式

1. C++17允许将一个二元操作符作用在参数包的所有参数上，分为一元左折叠、一元右折叠、带初始值二元左折叠、带初始值二元右折叠
2. 主要用在模板编程，特别是可变参数模板

```cpp
template <class...Args> //一元左折叠
void Print(Args...args) {
	(std::cout << ... << args);
}
template <class...Args> //一元右折叠
auto all(Args...args) {
	return (args && ...);
}
template<class...String> //二元左折叠
auto concatleft(String...str) {
	return (std::string("###") + ... + str);
}
template<class...String> //二元右折叠
auto concatright(String...str) {
	return (str + ... + std::string("###"));
}
```

如果操作符是逗号的话，因为逗号表达式严格从左向右执行的特点，所以左右折叠的选择不会有影响，但是对于其他来说，比如上面`string`的例子，就会使`###`分别出现在左右两侧

3. 应用举例

```cpp
template<class...Args> //使用逗号运算符和lambda表达式来给打印添加空格
void PrintWithSpace(Args...args) {
	auto space = [=](int x) {std::cout << x << " ";};
	(..., space(args));
	std::cout << std::endl;
}
template<class V, class...Vals> //给数组尾插多个元素
void Push_vals(V& v, Vals...vals) {
	(..., v.push_back(vals));
}
```

4. 传空参数包的情况

* `&&`，true
* `||`，false
* `,`，void()
* 带初始值的二元折叠，init

### 类模板参数的自动推导

1. C++17之前需要手动指定模板类型，例如`vector<int> v`，但是C++17之后可以自动识别

```cpp
	std::vector v1{ 1 };
	std::vector v2{ "hello", "world" };
	std::vector v3(10, 1);
	//std::vector v4; //报错，无法推导类型
	std::pair p(1, "zhangsan");
```

### 非类型模板参数

1. 非类型模板参数是指在`template`后面写的诸多参数中，类型已经指定了的(即没有`typename` `class`修饰)模板参数
2. C++17之前，要求非类型模板参数的类型必须严格指定，但是C++17允许使用`auto`来自动识别非类型模板参数的类型，但是仅限于整形、枚举值、指针、左值引用、`nullptr_t`；后面到C++20，又支持了浮点数和字面量类类型（后面到C++20在详细说明，现在可以简单理解为在编译期就能完成构造、析构的简单的类，比如下面的Color和Point）

```cpp
template <auto Value>
void PrintValue() {
	std::cout << typeid(Value).name() << std::endl;
}

enum Color {
	RED,
	BLACK,
};

struct Point {
	int x;
	int y;
};

PrintValue<1>(); //整形
PrintValue<RED>(); 
PrintValue<nullptr>(); //nullptr_t
PrintValue<'a'>(); //指针
PrintValue<true>();

PrintValue<1.1>();
constexpr std::array<int, 2> arr(1, 2);
PrintValue<arr>();
constexpr Point pos(1, 2);
PrintValue<pos>();
```

#### 嵌套命名空间

1. C++17之前，定义嵌套命名空间的方式

```cpp
namespace a{
    namespace b{
        namespace c{
            int x = 0;
        }
    }
}
```



2. C++17的小语法糖

```cpp
namespace a::b::c{
    int y = 0;
}
```

3. 为了简化指定命名空间访问时的步骤，可以采取以下方式定义并访问

```cpp
namespace abc = a::b::c;
std::cout << a::b::c::x << " " << abc::y << std::endl;
```

### __has_include

1. 用来检查某个头文件是否存在并且可以被包含，既可以检查库的，也可以检查自己的头文件

```cpp
#if __has_include(<filesystem>)
	#include <filesystem>
#elif __has_include(<experimental/filesystem>)
	#include <experimental/filesystem>
#else
	#error "<filesystem> needed"
#endif

#if __has_include("header.h")
	#include "header.h"
#endif
```

### 属性

1. C++11引入了属性，在此之前主流编译器都有自己的实现方法，但是在C++11时只引入了两个属性

* `[[noreturn]]`告知编译器不会有返回值，主要用于优化和错误检测
* `[[carries_dependency]]`专家级别的并发优化，但对于99.9%的开发者，使用`atomic`的默认内存序以及其他高级并发工具已经足够

```cpp
//终止程序的函数
[[noreturn]] void fatal_error(const std::string& message) {
	std::cout << message << std::endl;
	exit(EXIT_FAILURE);
	return;
}

//用来抛异常的函数
[[noreturn]] void throw_error() {
	throw std::logic_error("...");
}

//执行死循环的函数
[[noreturn]] void server_start() {
	while (1) {

	}
}
```

* C++14引入了`[[depracated]]`，用来接口转换提示

```cpp
[[depracated("this func has been depracated, use new_func instead")]] int old_func(){
    //...
}
```

* C++17引入了`[[fallthrough]]` `[[nodiscard]]` `[[maybe_unused]]`

```cpp
void func(int op) {
	switch (op) {
	case 1:
		//...
		[[fallthrough]]; //告诉编译器此处不break是有意为之
	case 2:
		//...
		break;
	default:
		//...
		break;
	};
}

[[nodiscard]] int func_1() {  //告知编译器此处返回值不应被丢弃，否则报警告。C++20后支持说明原因
	return 1;
}

int func_2([[maybe_unused]]int val) {  //告知编译器这个变量可能不会被使用，不要报变量未使用的警告
	return 1;
}

```

* C++20引入了`[[likely]]` `[[unlikely]]` `[[no_unique_address]]`；`[[likely]]`和`[[unlikely]]`用来向编译器指示分支路径中具有极大概率偏差的两条路径，编译器可能会根据指示生成更高效的机器代码；`[[no_unique_address]]`，用来向编译器指示空对象不需要开辟空间（C++默认的是空对象为了标识存在也要开一个字节的空间，加上可能存在的）

```cpp
void options(int val) {
	if (val == 1)[[likely]] {
		//...
	}
	else if (val == 2) [[unlikely]] {
		//...
	}
}

class Empty {};

class C {
	int a = 0;
	[[no_unique_address]] Empty e;
};

```

### optional

1. `std::optional<T>`表示其可能包含一个`T`类型的值，也可能没有；可以用来接受错误结果的同时，也能够传递正确结果
2. 创建`optional`对象

```cpp
	std::optional<int> int_opt(1);
	std::optional<std::string> str_opt("hello world");
	std::optional<double> dou_opt(1.11);
```

3. 判断是否有值

```cpp
if (int_opt.has_value()) {
	std::cout << int_opt.value() << std::endl;
}
if (str_opt) {
	std::cout << str_opt.value() << std::endl;
}
```

4. 访问对象

```cpp
	try {
		int val_dou = dou_opt.value();
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	int val_opt = int_opt.value_or(2);
```

`value_or`是如果内部有值的话，就返回内部的值，没有的话就返回给予的参数

5. 修改对象

```cpp
	int_opt = 2;
	str_opt = std::nullopt; //置空
	dou_opt.reset(); //置空
```

### variant

1. variant允许一个对象可能是多种类型之一的变量，可以认为是一个高级的联合体
2. **variant的创建与赋值**，给variant赋不同的值，就可以自动识别存储指定的类型之一

```cpp
std::variant<int, double, std::string> v;
v = 1; v = 1.1; v = "hello world"; v = 'a';
std::cout << v.index() << std::endl; //获取当前存储类型在指定类型中的下标，例如0代表int
```

* variant在默认初始化时，会成为第一个类型，所以需要保证第一个类型是自定义类型的话必须有默认构造函数，也可以将第一个类型设置为`std::monostate`

```cpp
struct A{A(int a): _a(a){} int _a;};
std::variant<std::monostate, A, int> var;
```



* variant中不能存储数组、void、引用
* variant可以存放相同的类型，但是赋值时需要使用`emplace`，指定需要存储的具体是那个类型

```cpp
	std::variant<int, int> var;
	var.emplace<0>(1);
```

3. **variant的访问**

* 通过直接get，如果指定的类型和实际存储的不一样的话会抛异常

```cpp
	try {
		std::cout << std::get<2>(v) << std::endl; //通过类型编号
		std::cout << std::get<std::string>(v) << std::endl; //通过类型
	}
	catch (const std::bad_variant_access& err) {
		std::cout << "error: " << err.what() << std::endl;
	}
```

* 通过get_if，如果匹配的话，返回存储的值的指针，否则返回空指针，指定哪个variant时，也要使用指针的方式传入

```cpp
	using Variant = std::variant<int, double, std::string>;
	std::vector<Variant> vas = { 1, 1.1, 'a', "hello world" };
	for (auto& e : vas) {
		if (auto ptr = std::get_if<int>(&e)) { 
			std::cout << *ptr << std::endl;
		}
		if (auto ptr = std::get_if<double>(&e)) {
			std::cout << *ptr << std::endl;
		}
		if (auto ptr = std::get_if<std::string>(&e)) {
			std::cout << *ptr << std::endl;
		}
	}
```

* 通过visit，使用可调用对象 + variant的方式，让variant中存储的值直接由可调用对象处理

```cpp
//通过lambda表达式
for (auto& e : vas) {
	std::visit([=](auto&& e) {std::cout << e << std::endl;}, e);
}
```

对于需要根据类型进行各自处理的情况，可以使用以下三种方法

* 通过仿函数

```cpp
	struct variantOP {
		void operator()(int& data) {
			std::cout << "int: " << data << std::endl;
		}
		void operator()(double& data) {
			std::cout << "double: " << data << std::endl;
		}
		void operator()(std::string& data) {
			std::cout << "string" << data << std::endl;
		}
	};
	for (auto& e : vas) {
		std::visit(variantOP(), e);
	}
```



* 通过模板元编程

```cpp
	for (auto& e : vas) {
		std::visit([=](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<int, T>) {
				arg *= 2;
				std::cout << "variant doubled: " << arg << std::endl;
			}
			else if constexpr (std::is_same_v<double, T>) {
				arg *= 2.0;
				std::cout << "variant doubled: " << arg << std::endl;
			}
			else if constexpr (std::is_same_v<std::string, T>) {
				arg += arg;
				std::cout << "variant doubled" << arg << std::endl;
			}
			else {
				static_assert(false, "unknown variant type");
			}
		}, e);
	}
```

* 这种方式比较晦涩难懂，但是可能比较简洁一些

```cpp
	template <class...lambda>
	struct overload : lambda...{
		using lambda::operator()...;
	};

	for (auto& e : vas) {
		std::visit(overload{
			[=](int val) {std::cout << "int: " << val << std::endl;},
			[=](double val) {std::cout << "double: " << val << std::endl;},
			[=](std::string val) {std::cout << "std::string: " << val << std::endl;},
		}, e);
	}
```

> lambda表达式本质上是仿函数，此处通过让overload类继承所有指定仿函数的方式，就等于让这个类重载了所有lambda仿函数，其实本质上也是第一种通过仿函数的方式，只不过前面的是直接手写，这里是使用传参时指定

4. variant的实际应用

​	这里举一个例子，哈希桶，实现某个尾置内容少时采用链表，多时采用红黑树

```cpp
template <class...lambda>
struct overload: lambda... {
	using lambda::operator()...;
};

template <class T>
class HashTable {
public:
	const int TABLESIZE = 11; //哈希桶的大小
	using Value = std::variant<std::list<T>, std::set<T>>;
	HashTable()
	{
		_table.resize(TABLESIZE);
	}

	void insert(T& val) {
		int hashi = val % TABLESIZE;
		auto list_insert = [&val, &hashi, this](std::list<T>& lt) {
			int size = lt.size();
			if (size <= 1) {
				std::cout << "lt insert: " << val << std::endl;
				lt.push_back(val);
			}
			else {
				std::cout << "set insert: " << val << std::endl;
				std::set<T> new_con(lt.begin(), lt.end());
				new_con.insert(val);
				_table[hashi] = std::move(new_con);
			}
		};
		auto set_insert = [&val](std::set<T>& s) {
			std::cout << "set insert: " << val << std::endl;
			s.insert(val);
		};
		std::visit(overload{ list_insert, set_insert }, _table[hashi]);
	}

	void find(int key) {
		int hashi = key % TABLESIZE;
		auto list_find = [&key](std::list<T>& lt) {
			if (auto it = std::find(lt.begin(), lt.end(), key); it != lt.end()) {
				std::cout << "list find: " << key << std::endl;
				return true;
			}
			return false;
		};
		auto set_find = [&key](std::set<T>& s) {
			if (auto it = std::find(s.begin(), s.end(), key); it != s.end()) {
				std::cout << "set find: " << key << std::endl;
				return true;
			}
			return false;
		};
		std::visit(overload{ list_find, set_find }, _table[hashi]);
	}
private:
	std::vector<Value> _table;
};

```

### Any

1. 可以用来存储任意的类型，比variant存储的更加灵活，而且在进行`any_cast`时会进行类型检查
2. 同时，编译器也会进行SBO(Small Buffer Optimization)，Any本身就在栈上，同时也会在栈上申请一定的缓冲区，对于能够在栈上存下的类型（不止要考虑绝对大小，也要考虑内存对齐问题），就存储在这个缓冲区中；否则就存储在栈上（栈的操作效率比堆要快一两个数量级）
3. Any会管理内部对象的生命周期，存储时会构造，当修改存储对象的时候会析构释放原来存储的对象

#### 1. 构造和赋值

```cpp
	std::any a2 = 2.2;
	std::any a3 = std::string("hello");  //自动识别类型
	a3.emplace<std::string>("world");  //替换的时候需要指定类型
	if (a3.has_value()) std::cout << "a3 has value" << std::endl;  //通过has_value去判断是否存储值
	const std::type_info& t3 = a3.type();  //获取类型
	std::cout << t1.name() << " " << t2.name() << " " << t3.name() << std::endl;
```

#### 2. any_cast

* 转换为值，这种方式访问获取到的是any中值的拷贝

```cpp
std::any a1 = 1;
int val = std::any_cast<int>(a1); //类型不匹配会抛异常
```

* 转换为引用，可以是左值，也可以是右值走移动构造

```cpp
std::any a1(std::string("hello")); std::any a2(std::string("world"));
std::string& ra1 = std::any_cast<std::string&>(a1);
std::string ra2 = std::any_cast<std::string&&>(std::move(a2));
```

* 转换为指针，传入的是any的指针；类型匹配，返回存储的值的指针，否则返回空指针

```cpp
std::any a = 1;
if(auto ret = std::any_cast<int>(&a); ret != nullptr){
    std::cout << *ret << std::endl;
}
```

### 3. any 和 variant对比

1. any和variant都可以在构造或之后赋值；any的访问使用`any_cast`，variant的访问使用`visit` `get` 等，前者相对简洁明了一些

2. any的存储在栈中或者在堆中，variant的在栈中，后者相对更快一些
3. variant在编译时已知所有可能类型，any要等到运行期；访问时any也需要进行类型查询和识别转换等，访问的效率也相对慢一些

### string_view

1. `string_view`是C++17引入的对已有的字符串或数组进行查看而不需要拷贝的类，简单理解就是`string_view`是观察一个目标的“望远镜”，不负责管理资源，只保存原始字符串的指针和长度，`const string_view`只能查看，不能修改
2. `string_view`的构造和析构几乎是零开销的，而且几乎提供了所有`std::string`的读接口

```cpp
	std::string_view sv1 = "hello world"; //从C风格字符串构造

	std::string str = "string_view-read-string";
	std::string_view sv2(str); //从string构造

	std::string_view sv3(str.c_str() + 6, 6); //从部分字符串构造（从string的第六个字符开始，取六个字符）

	using namespace std::literals; //使用库提供的字面量后缀
	std::string_view sv4 = "hello world"sv; //从字面量构造
```

#### string_view的陷阱

1. 使用已经被释放的字符串的视图

```cpp
std::string_view get_view() {
	std::string temp = "hello world";
	return temp; //这里返回的string_view指向的是栈上的字符串
}

void error_example1() {
	auto sv = get_view();
	std::cout << sv << std::endl;
}
```

2. `data()`返回的内容不一定包含`\0`，所以要注意保证内容是包含`\0`的

```cpp
	char filename[] = { 't', 'e', 's', 't', '.', 'c', 'p', 'p' }; //不以'\0'结尾的字符数组
	std::string_view sv(filename);
	std::cout << sv.data() << std::endl; //data()成员函数返回的不一定是以'\0'结尾的原始内容
```

解决方法，构造C风格字符串或者在构造`string_view`时指定长度并且访问时不使用`data()`

2. 因为`string_view`中保存的是原始字符串的指针和长度，所以当原始字符串的内容发生改变时，可能会导致`string_view`失效

```cpp
	std::string s = "hello";
	std::string_view sv(s);
	s = "1111111111111111111111111111111111111111111111111111111111111111111111111111";
	std::cout << sv << std::endl; //sv中保存着指针，但是原始的字符串已经改变了位置
```

### filesystem

1. `std::filesystem`是C++17提供的跨平台的文件系统接口，对于mac、windows、linux等平台都可以忽略底层细节的使用
2. 命名空间较长，可以通过别名访问 

> namespace fs = std::filesystem

#### 路径对象的构造

```cpp
fs::path p1 = R"(C:\Windows\System32)"; //对于Windows来说，因为路径分隔符是转义字符，所以要\\，就比较麻烦，可以通过这种方式自动识别反斜杠转义
fs::path p2 = fs::current_path(); //获取当前路径
```

#### 路径信息获取

```cpp
p1.root_name(); //根目录名称
p1.root_directory(); //根目录
p1.root_path(); //根目录路径
fs::current_path(); //当前目录名称

p1.parent_path(); //父路径名称
p1.relative_path(); //相对路径
p1.stem(); //不带后缀名的文件名
p1.filename(); //文件名
p1.extension(); //后缀名
```

#### 路径修改

```cpp
p1 /= "GitFiles"; //通过'/='来对文件名末尾修改
p1 /= "moderncpp";
p1 /= "17";
p1 /= "info.md";
 
p1.replace_filename("test.cpp"); //添加末尾文件名
p1.remove_filename(); //删除末尾文件名
```

其它部分的修改也是类似

#### 路径的检查与转换

```cpp
	if (p1.is_absolute()) {
		std::cout << "p1 is absolute" << std::endl;
	}
	else if(p1.is_relative()) {
		std::cout << "p1 is relative" << std::endl;
	}

	if (p1.has_filename()) {
		std::cout << "p1 has filename" << std::endl;
	}
	else if (p1.has_root_name()) {
		std::cout << "p1 has root name" << std::endl;
	}

	auto s1 = p1.string(); //转换为平台依赖的string
	auto s2 = p1.wstring(); //宽字符
	auto s3 = p1.u8string(); //utf8
```

#### 权限操作

1. 对于Unix类系统，这里的权限操作要可靠很多，因为和`filesystem`的操作逻辑一样，都是位运算，相比之下Windows的文件权限要复杂很多，虽然也支持但是不如Linux等下可靠
2. 文件的权限在`std::filesystem::perms`中，大类是`owner` `group` `others`，其中又有`read` `write` `exec`三种具体权限；权限的获取通过`std::filesystem::status()`中的`permissions()`获取；操作通过`std::filesystem::permissions`，主要有`replace` `add` `remove`三种操作

```cpp
    try{
        fs::path file_path("test.txt");
        std::ofstream(file_path) << "hello world";
        auto status = fs::status(file_path).permissions();

        fs::permissions(file_path, fs::perms::owner_exec | fs::perms::group_exec, fs::perm_options::add);
        status = fs::status(file_path).permissions();

        fs::permissions(file_path, fs::perms::group_exec, fs::perm_options::remove);
        status = fs::status(file_path).permissions();
    }
    catch(const std::exception& e){
        std::cout << e.what() << std::endl;
    }
```

