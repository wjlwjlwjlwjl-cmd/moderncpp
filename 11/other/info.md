## constecpr和常量表达式

### 1. 底层const和顶层const

1. 对于大部分对象，被const修饰都是顶层const；
2. 对于指针来说，本身被const修饰（*在const左边）叫做顶层const；指向对象被const修饰（*在const右边）叫做底层const
3. 对于引用，被const修饰的是顶层const

### 2. constexpr

1. 常量表达式是指值不会改变而且编译期就能确定结果的变量，使用字面量、const表达式初始化的const变量都是常量表达式，使用变量初始化的const变量不是
2. constexpr用来修饰的变量必须是常量表达式，同时也必须使用常量表达式初始化，如果不是的话编译期报错，constexpr也会让编译器对所修饰常量表达式进行优化，提升运行时效率。比如将const变量放到寄存器当中，取得时候直接从寄存器而不从内存当中去取
3. constexpr也可以修饰指针，const修饰的指针是指针本身，即顶层const
```cpp
constexpr const int a = 0;
constexpr const char *pstr = "hello world";
```
## constexpr函数

### constexpr在C++11 -> C++14 -> C++20的演进过程

#### C++11

返回值类型单一，内部只支持一句return语句，想要执行多个步骤只能递归

#### C++14

1. 函数体内可以有多个语句，支持控制流（循环、判断等）语句
2. 支持返回值为自定义类型（constexpr自定义类型）以及STL容器如array（对于'[]'的支持到C++17）
3. 支持定义局部变量

#### C++17

1. 支持if constexpr分支语句，对于符合条件的编译器运算，其他的丢弃
2. 支持作用在lambda表达式，要求同对函数的要求，捕获的值也必须是字面量，constexpr位置在参数列表后返回类型定义前

#### C++20

1. 支持new delete，但是必须在constexpr函数体中完成资源的释放（包括使用STL容器，内部有动态资源的话）
2. 开始支持STL容器如vector，以及一些接口如find sort
3. 支持try-catch语句，但是并不是普通的运行时抛异常，而是异常出现时直接编译不通过
4. 支持constexpr联合体，外界可以访问其活跃变量
5. 支持mutable修饰constexpr类的成员变量以实现在其constexpr成员函数中进行修改该成员变量
6. 支持constexpr虚函数

### constexpr普通函数

1. constexpr也可以修饰函数，通过这种方式修饰的函数参数、返回值都只能是字面量，调用的时候也必须使用字面量或者常量表达式传参。
2. constexpr修饰的函数如果符合要求调用的话，那么就不会在运行时得到结果，比inline内联函数在运行时省去的工作更多，直接在编译时去完成函数结果的运算，虽然会导致编译时间变长；如果不符合调用要求的话就当作普通函数调用
```cpp
constexpr int fib(int a){
    return a == 1 ? 1 : a + fib(a - 1);
}
constexpr int ret = fib(100);
int n = 100; int ret = fib(n);
```
### constexpr构造函数

1. constexpr不能修饰自定义类型，除非该类型的构造函数被constexpr修饰，同时该自定义类型有以下要求
* 所有成员变量都是字面量，并且都在初始化列表中初始化
* 构造函数参数都是字面量
2. 如果没有按照要求传参，那么就是在运行期进行的普通构造

### constexpr成员函数

1. 当constexpr修饰的成员函数所在的自定义类型的构造函数是被constexpt修饰时，并且通过constexpr修饰的自定义类型调用，就可以同样在编译器就完成对自定义类型的构建和其constexpr成员函数的执行，其实使用和constexpr修饰的普通函数类似
2. 没有符合要求同样是按照普通成员函数执行

### constexpr模板函数

1. 其实就是constexpr普通函数加上了模板这一层，一起在编译期完成工作
```cpp
class Date
{
public:
    constexpr Date(int year, int month, int day)
        : _year(year), _month(month), _day(day){}
    constexpr int getYear() const{
        return _year;
    }
private:
    int _year; int _month; int _day;
};

template <class T>
constexpr int add(T t1, T t2){
    return t1 + t2;
}

int main(){
    constexpr Date d(1, 2, 3);
    std::cout << d.getYear();
    constexpr int ret1 = add<int>(1, 2);
    return 0;
}
```

### consteval

* constexpr修饰的函数，既可能在编译时求值，也可能在运行时求值，consteval修饰的函数要求必须在编译时求值，即满足constexpr修饰函数时须满足的条件，否则编译报错

```cpp
consteval int func(int a, int b){
    return a + b;
}
constexpr int a = func(1, 2);
```

### constinit

* constinit作用于变量，要求必须在编译时初始化，但是在初始化后并不作为常量，而是可以在运行时被修改
* constinit可以作用于静态存储区和线程存储期的变量，例如全局变量、static变量、thread_local变量（在各个线程内都存在一份的变量），不能作用于自动存储期的变量，例如函数的局部变量（主函数中的非全局、非静态变量也算）

```cpp
constexpr int func(int a, int b) {return a + b;}
consteval int func2(int a, int b) {return a - b;}
constinit int a = func(1, 2); constinit int b = func2(1, 2);
int main() {constinit static int c = 1; return 0;}
```

* constinit还可以作用在类上，保证其在编译时初始化

```cpp
class A{public: constexpr A(int a) :a(a){} private: int a};
constinit A a(1);
int main() {...}
```

* constinit还可以解决多个cpp文件中全局变量、静态变量初始化顺序的问题，例如以下代码

```cpp
//a.cpp
int a = 10; //全局

//b.cpp
extern int a;
int b = a;
```

这种方法就可能出现b在a之前初始化的问题，所以使用constinit就能使两者一定在编译器都完成初始化

```cpp
constinit int a = 10;
extern constinit int a;
constinit int b = a;
```

### auto

* auto用来自动推到类型，但是他有以下四点值得注意

1. auto不能推导出引用，想要使用引用必须显式指定。因为直接把引用对象交给auto的话，那么auto最后拿到的实际还是被引用的对象，那么就不会初始化出引用类型

```cpp
int a = 1; int& ra = a;
auto c = ra; //c的类型是int
```

2. 具有const属性的对象去初始化auto对象时，顶层const会被抛弃（修饰本身的const），底层const会被保留

```cpp
const int a = 10; 
auto b = a; //b的类型是int
const int* pa = &a;
auto c = pa; // c的类型是const int*
```

3. 使用auto& 去显式推导引用类型时，顶层const会被保留

```cpp
const int a = 10; 
auto& b = a; // const int&
const int* const pa = &a;
auto& c = pa; // const int* const
```

4. auto&&为万能引用，传左值为左值引用，传右值为右值引用，const属性保留

```cpp
int a = 10; const int c = a;
auto&& r1 = a; //int&
auto&& r2 = c; //const int&
auto&& r3 = std::move(a); //int&&
auto&& r4 = std::move(c); //const int&&
```

### 尾置返回类型和decltype

1. 尾置返回类型是在参数列表之后，通过箭头指定返回类型表达式，与auto配合使用，在C++11主要是为了解决auto不能直接作为返回类型的问题，不过之后的标准放开了这个限制，所以尾插表达式也就没那么重要了
2. decltype是可以返回变量表达式的类型，而且不像auto一样会对类型做处理，而是给什么类型最后的结果就是什么类型；也可以通过函数调用表达式的方式去使用函数调用返回值的类型，但是这个过程中并不会真正去调用函数
3. decltype对与引用有两种特殊的处理

```cpp
int a = 10; int* pa = &a;
decltype((a)) b = a;
decltype(*pa) c = a;
```

b和c的类型都是int&

4. auto 必须通过初始化值来完成类型推导，这一点对于成员变量是很不方便的，但是decltype就解决了这个问题

```cpp
template <typename T>
class It {
public:
	void func(T& container) {
		_it = container.begin();
	}
private:
	decltype(T().begin()) _it;
};
```

5. C++14中，结合auto自动类型推导和decltype精准的类型判断，可以使用decltype(auto)的方式自动精确推导

```cpp
int a = 10; const int& ra = a;
decltype(auto) b = ra; //b的类型是const int&
```

### using typedef

```cpp
typedef void(*)(int, int) func;
using func1 = void(*)(int, int);

template<class T>
using unordered_map<string, T> con;
```

1. typedef和using都用来重定义类型，但是两者有用法和功能上的区别。
2. typedef采取先是原类型再是重定义类型的顺序（或者嵌套，比如函数指针的定义）；using采取先是重定义类型再是原类型的方式
3. using可以完成带模板参数的类型重定义

### 强类型枚举

```cpp
enum Color {RED, BLUE, YELLOW}; //不能指定底层类型
enum class Gender :uint8_t {MALE, FEMALE};
int main(){
    int col2 = RED; //污染作用域、自动隐式类型转换
    Gender male = Gender::MALE;
}
```

1. 强枚举类型是对普通enum的优化，写为enum class或者enum struct
2. 强枚举类型可以保证只能通过指定类域的方式访问，能够避免隐式类型转换为int，也能够通过 :type的方式指定底层类型

### static_assert

```cpp
static_assert(sizeof(void*) == 8, "the platform should be 64-bit");
static_assert(sizeof(int) == 4, "the int type should has 4 bytes")
```

1. static_assert是用来做编译时断言，如果不通过则编译报错，与之相对的是assert，它是在运行时报错并终止运行
2. static判断的式子必须是常量表达式，即在编译期间可以确定结果

### tuple

1. 可以理解为支持任意多个类型的pair，C++17之后还支持了自动类型识别，不需要显式指定类型

```cpp
tuple<int, double, string> t1 = {1, 1.1, "1.11"};
auto t2 = make_tuple(2, 2.2, "2.22");
```

2. 根据下标获取tuple元素

```cpp
int a = get<0>(t1);
double b = get<1>(t2);
string str = get<2>(t1);
```



3. tuple的解包可以通过tie，但是C++17之后可以使用结构化绑定，更加方便

```cpp
std::tuple t1(1, 1.1, "1.11");
int a; double b; string str;
std::tie(a, b, str) = t1;

auto [val1, val2, val3] = t1;
```

### 模板元编程

1. 利用模板的递归实例化等等语法规则，在编译器就完成一部分结果的运行获取，常用类型变量或者静态成员变量来完成

```cpp
//判断两个类型是否相同
template <class T, class U>
struct is_same {
    constexpr static const bool value = false;
};
template <class T>
struct is_same<T, T> {
    constexpr static const bool value = true;
};

//去掉const
template <class T>
struct remove_const {
    using type = T;
};
template <class T>
struct remove_const<const T> {
    using type = T;
};
```



### 类型萃取

1. 其实上面的例子中已经是简单的类型萃取了，不过在STL中也给出了实现
2. C++17后，为了简化书写，用`_v` `_t`来代替`::value` `::type`
3. 进行基础类型判断

```cpp
	std::cout << std::is_void<void>::value << std::endl;
    std::cout << std::is_integral<int>::value << std::endl;
    std::cout << std::is_floating_point<float>::value << std::endl;
    std::cout << std::is_pointer<int*>::value << std::endl;
    std::cout << std::is_reference<int&>::value << std::endl;
    std::cout << std::is_const<const int>::value << std::endl;
```



4. 进行复合类型检查

```cpp
    std::cout << std::is_function_v<void()> << std::endl; //is function?
    std::cout << std::is_member_object_pointer_v<int (Foo::*)> << std::endl; //is object's member pointer
    std::cout << std::is_compound_v<std::string> << std::endl; //is fundamental type
```

5. 进行关系检查

```cpp
	std::cout << std::is_same<int, int32_t>::value << std::endl;
    std::cout << std::is_base_of_v<Base, Derive> << std::endl;
    std::cout << std::is_convertible<char*, std::string>::value << std::endl; //前者能够转化为后者
```

6. 类型修改

```cpp
	int n = 10;
    const int cn = 10;
    std::add_const<int>::type a = 1;
    std::add_pointer_t<int> pa = &n;
    std::add_lvalue_reference<int>::type rn = n;
    std::remove_const<decltype(cn)>::type b = 10;
    std::remove_pointer_t<int*> c = 10;
    std::remove_reference<int&>::type d = 10;
```

7. 条件类型选择

```cpp
    std::cout << std::is_same_v<int, std::conditional_t<true, int, float>> << std::endl;
    std::cout << std::is_same_v<float, std::conditional_t<false, int, float>> << std::endl;
```



8. 模板类型推导

```cpp
template <class F, class...Arg>
using invoke_result_t = std::invoke_result_t<F, Arg...>()
```

### SFINAE

1. SFINAE并不是单词，而是substitute failure is not an error，当模板匹配失败的时候，并不会直接编译报错，而是寻找是否有其他合适的模板，如果找到最后也没有那么就编译报错
2. 应用一：函数重载

```cpp
template <class T>
auto func(T x) -> decltype(x++, void()) {
	std::cout << "x is implementable" << std::endl;
}
template <class T>
auto func(T x) -> std::void_t<decltype(x.size())> {
	std::cout << "x has member fucntion size" << std::endl;
}
void func(...){
    std::cout << "fallback func" << std::endl;
}
```

在上面的decltype中，会首先尝试执行第一个条件（例如x是否可以++，x是否具有成员函数size() )，当条件成功时，decltype会继续往后执行逗号表达式并返回类型对象（比如void对象），当条件失败时，会继续寻找其他模板，都没有就会调用func(...)



3. 应用二：enable_if，在编译时启用或者禁用函数模板

```cpp
template <class T>  //对int类型启用
typename std::enable_if_t<std::is_integral<T>, int> Foo(T t){
    return t + 1;
}
template <class T>  //对浮点数类型启用
typename std::enable_if_t<std::is_floating_point<T>, float> Foo(T t){
    return t / 2;
}
```

enable_if会在第一个模板返回true时，给出第二个参数的类型，不过有一个void类型的缺省值，不写的话在上面的例子中就是返回void

4. 应用三：模板类型检查

```cpp
template <class K, class V = std::enable_if_t<std::is_integral<K>>>
void func(K k){
    std::cout << ++k << std::endl;
}
```

如果enable_if判断条件失败不返回类型的话，V就会推导失败，因为V的作用就是进行类型检查，所以不写名称也可以
