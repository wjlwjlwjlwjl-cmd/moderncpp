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

