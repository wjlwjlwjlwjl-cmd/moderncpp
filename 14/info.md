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

