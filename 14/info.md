### 变量模板

1. C++14不仅允许类、函数模板，也允许了变量模板

```cpp
//根据类型控制精度
template <class T>
constexpr T pi = T(3.1415926535897932);
template <class T>
T cirS(T r) {
	return pi<T> *r * r;
}
```

2. 可以通过递归的实例化完成某种类型的递归运算

```cpp
template <size_t N>
constexpr size_t factorial = N * factorial<N - 1>;
template <>
constexpr size_t factorial<0> = 1;
```

3. C++17中对类型萃取优化写法的简单实现

```cpp
template <class T>
constexpr bool is_integral_v = std::is_integral<T>::value;
```

### 泛型lambda

1. 支持参数列表中使用`auto`去自动判断类型，也支持引用折叠的语法规则

```cpp
auto func = [](const auto& a, const auto& b) {
	return a > b? a: b;
};
auto ref = [](auto&& r1, auto& r2) {};
```

2. 支持可变参数模板

```cpp
std::vector<std::string> v;
auto pushs = [&v](auto&&...ts) { v.emplace_back(std::forward<decltype(ts)>(ts)...);};
```

3. 支持在捕获列表中初始化值，可以是当前作用域中的值，也可以是新的值

```cpp
auto p = std::make_unique<int>(10);
std::vector<int> v;
auto lambda1 = [value = 5, ptr = std::move(p), v]() {
	std::cout << value << " " << *ptr << " " << v.size() << std::endl;
};
```

4. 在C++20之后，还给泛型lambda加上了类似模板参数的语法，在捕获列表和参数列表之间

```cpp
vector<int> v;
auto pushs_model = [&v]<typename...Arg>(Arg...args){v.emplace_back(std::forward<Arg>(args)...)};
```

### 函数返回类型推导

1. C++11中，auto不能直接做返回类型，一般是要和尾置返回类型配合使用的

```cpp
int x = 10;
auto f1() -> int { return x; }; 
auto f2() -> int& { return x; }; 
auto f3(int x) -> decltype(x * 1.5) { return x * 1.5; }; 
```

需要注意的是，这里的auto只能写为简单的auto，不能像下面一样根据auto推导的规则做出自定义（因为已经写了尾置返回类型）

2. C++17之后，可以直接使用auto作为返回类型

```cpp
int y = 10;
const int z = 10;
auto f4() { return y; }; //int
auto& f5() { return y; }; //int&
auto f6() { return z; }; //int，auto推导时，顶层const会被舍弃
```

这里其实就是按照auto推导的四个规则去处理即可

3. 如果一个函数有多个返回语句，且使用auto直接推导返回类型，那么各个返回语句返回类型必须相同（使用`if constexpr`除外，因为不符合条件的语句会被直接舍弃）

```cpp
template <class T>
auto f9(T x) {
	if constexpr (std::is_integral_v<T>) {
		return 1.1;
	}
	else{
		return "hello";
	}
}
```

4. `decltype(auto)`可以用来进行精确类型推导，保留顶层const，不会像auto一样对类型推导进行自己的处理

```cpp
int* py = &y;
decltype(auto) f7() { return (x); };
decltype(auto) f8() { return *py; };
decltype(auto) f10() { return (z); };
```

这里依然遵循`decltype(auto)`的两个特殊规则

5. `decltype(auto)`最常见的用法是完美转发返回类型的自动推导

```cpp
template <class F, class...Arg>
decltype(auto) call(F&& f, Arg&&...args) {
	return std::forward<F>(f)(std::forward<Arg>(args)...);
}
```

### 二进制字面量

1. C++14之后，引入了二进制字面量，就是可以直接像十进制、十六进制等直接通过字面量而不是字符串等形式给出。以`0b`或者`0B`开头

```cpp
int a = 10; //十进制
int b = 052; //八进制
int c = 0x12; //十六进制
int d = 0b1010; //二进制
```

#### 二进制字面量的应用

1. 位标志

```cpp
const int FLAG_A = 0b001;
const int FLAG_B = 0b010;
const int FLAG_C = 0b100;
```

2. 位图

```cpp
std::bitset<10>(std::string("10101"));
std::bitset<10>(0x12);
std::bitset<10>(0b1001);
```

m没有二进制字面量以前，只能通过其他进制数字的转换以及通过字符串给出的方式，两者都不方便，所以有二进制字面量之后就可以很方便的初始化位图



### 数字分隔符

1. 为了提升代码可读性，允许在表示数字时在任意位置加上`'`来表示分割意义，不影响数字的值

```cpp
int BIG_NUM = 123'456'789;
double pi = 3.141'592'653;
int HEX_VAL = 0x123'456'789;
```

### 使用默认非静态成员初始化器的聚合类

1. What's 聚合类

(1) 没有用户指定的构造函数（使用 `delete` `default`的不算）

(2) 没有私有或者受保护的成员

(3) 没有基类

(4) 没有虚函数

2. 默认成员初始化器：当构造聚合类时没有指定某些成员的值，就使用其声明时给定的初始值来完成初始化
3. 初始化顺序：按照聚合类中声明的顺序分配聚合初始化的值

```cpp
struct Foo{
    std::string s = "hello world";
    int a = 10;
    double b = 1.1;
};
Foo foo{"hello", 19}; //s = "hello", a = 19, b = 1.1
Foo foo1; //s = "hello world",  a = 10, b = 1.1
```

4. C++20之后，进一步放宽了限制，可以通过指定具体成员的方式类特定初始化某个成员，其他的使用默认初始化器初始化，但是这里的构造函数即使是使用`delete` `default`修饰的也不行

```cpp
struct Foo{
    int a = 10;
    int b = 11;
    int d=  13;
}
struct Bar{
    Foo foo;
    int c = 12;
}
Foo foo{.a = 0, .d = 0};
Bar bar{.foo = {.a = 1, .b = 2}, .c = 3}; //允许嵌套类的定义
```

### exchange

1. 方便的交换值。对于左值会调用拷贝赋值，右值会调用移动赋值

```cpp
	std::vector<int> v1{ 10, 20, 30 };
	std::vector<int> v2 = std::exchange(v1, { 1, 2, 3 });
	for (auto& e : v1) std::cout << e << " ";
	std::cout << std::endl;
	for (auto& e : v2) std::cout << e << " ";

	for (int a{ 0 }, b{ 1 }; b <= 10; a = std::exchange(b, a + b)) {
		std::cout << a << ", ";
	}
```

 	exchange的行为可以用以下代码模拟

```cpp
template <class T, class U>
T exchange(T& obj, U&& new_val){
    T old_val = std::move(obj);
    new_val = std::forward<U>(new_val);
    return old_val;
}
```



2. 关于类型的问题

```cpp
template <class T, class U = T>
T exchange(T& obj, U&& new_value){
    //...
}
```

`new_value`的类型给了一个`T`的类型作为缺省值，所以上面第一个数组的例子是可行的

### make_unique

1. 相比于直接`new`然后使用`unique_ptr`接管，`make_unique`更加安全，因为前者可能会出现在被`unique_ptr`接管之前构造异常导致资源未能释放，但是后者在内部会在构造抛异常时释放已经申请的资源
2.  `make_unique`会对申请的空间进行初始化
3. C++20之后引入了`make_unique_for_overwrite`，相比于`make_unique`，它不会对资源进行初始化，由用户完成对这些资源的初始化，相对来说节省了一定的性能

```cpp
template <class OutputInt>
void fibnnaci(OutputInt start, OutputInt end) {
	for (int a = 0, b = 1; start != end; start++) {
		a = std::exchange(b, a + b);
		*start = a;
	}
}

int main() {
	std::unique_ptr<int> up1 = std::make_unique<int>(1);
	std::unique_ptr<int[]> up2 = std::make_unique<int[]>(2);
    
	std::unique_ptr<int[]> up3 = std::make_unique_for_overwrite<int[]>(10);
	fibnnaci(up3.get(), up3.get() + 10);

	return 0;
}
```

