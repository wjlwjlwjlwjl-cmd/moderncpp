## 1 三路比较运算符（太空船运算符）
### 1.1 基本概念
C++20开始，允许使用 `<=>` 代替 `>` `<=` `==` 等符号，这意味着编译器会尝试自动为我们进行这些比较或者生成相应的运算符重载。
返回的结果是强枚举类型（enum class，不可以与整形进行相互转化）`strong_ordering` `partial_ordering` `weak_ordering` 之一
返回的结果支持与 0 比较。上述三个类型都实现了类似于 `ret < 0` 的运算符重载。这样，通过其返回值，就可以确定比较的结果了，例如
```cpp
std::string str1 = "hello", str2 = "hello";
auto ret = str1 <=> str2;
if(ret > 0) std::cout << "str1 > str2";
else if(ret < 0) std::cout << "str1 < str2";
else std::cout << "str1 == str2";
```
### 1.2 类中三路比较运算符的重载
类中可以根据自己的需要，重载 `<=>` 运算符，函数原型如下
```cpp
auto operator<=>(const ClassName& other) const{
}
```
返回的类型，是 `strong_ordering` `weak_ordering` `partial_ordering` 之一。`<=>` 运算符重载也可以交给编译器生成，编译器会依次比较成员变量（按照声明顺序），但需要 `ClassName` 中的成员都支持 `<=>`
```cpp
auto operator<=>(const ClassName& other) const = default;
```
因为 `<=>` 将比较的活交给了编译器会有编译时间开销以及编译器生成的版本无法自定义，所以会优先调用自己重载的比较运算符，如果没有的话就自己生成。
> string 中，现在就移除了除 `==` 以外的运算符重载，都交给编译器区自动生成，`==`没有删除的原因是这里面有 string 自己的比较逻辑：先比较长度，再比较字典序
### 1.3 比较类别
#### 1.3.1 strong_ordering（强序）
意味着两个值不仅仅是值上的相等，在意义上也相等，可以直接进行交换，是最严格的比较方式，内部包括四种静态值
* `equal、equivalant`，代表比较结果相等，两者在语义上是等价的
* `less`，代表小于，等价于 `arg1 < arg2`
* `greater`，代表大于，等价于 `arg1 > arg2`
#### 1.3.2 weak_ordering (弱序)
比前面的要弱，值相等的情况下，不保证两者是完全等价的，不一定可以进行交换，内部包括三种静态值
* `equal`，这里可以理解为对应到约等于
* `less`，小于，等价于 `arg1 < arg2`
* `greater`，大于，等价于 `arg1 > arg2`
弱序可以用来实现我们自己的比较逻辑，比如忽略大小写的字符串比较
#### 1.3.3 partial_ordering（偏序）
在我们的比较的值的范围中，是出现一些无法比较的值或者压根不存在的值的，比如浮点数的 `NAN(Not a Number，表示未定义或者无法表示的常数结果)`。`partial_ordering` 允许存在不可比较的情况，使用 `unordered` 静态值来表示，其他三个静态值和 `weak_ordering` 相同
### 1.4 <=> 的其他使用方式
其实仁者见仁，下面两种方式感觉不是特别的好用
#### 1.4.1 is_xxx
主要包括以下几种
* `is_eq` `is_neq`，is equal or is not equal，等价于 `(x <=> y) == 0` `(x <=> y) != 0`
* `is_gt` `is_gteq`，等价于 `(x <=> y) > 0` `(x <=> y) >= 0`
* `is_lt` `is_lteq`，等价于 `(x <=> y) < 0` `(x <=> y) <= 0`
#### 1.4.2 compare_three_way
定义一个 `compare_three_way`对象，将要比较的值交给它，就等价于直接使用 `<=>`
## 2 format库
### 2.1 format
其实可以认为是一个 Plus 版的 printf，返回一个 string，接受一系列的参数，使用 `{}` 作为占位符
#### 2.1.1 位置参数
有两种方式，一种是采用默认的方式
```cpp
auto str = std::format("name: {}, age: {}", name, age);
```
就和 printf 一样从前往后匹配，不匹配的话回报错
第二种方式是位置索引
```cpp
auto str = std::format("name: {1}, age: {0}", age, name);
```
从下标 0 开始。如果数量不匹配报错，如果位置索引、默认索引混用，报错
#### 2.1.2 格式化参数
这里挺多挺杂的，这里主要写几个常用的。总的来说，占位符结构基本如下 `{:[arg_id][format_spec]}`，其中 `arg_id` 是位置参数，`format_spec` 是格式说明符
**精度** 
```cpp
std::string str1 = std::format("the val is {:.2f}", pi); //浮点数精度
std::string str2 = std::format("the str is {:2f}", "hello"); //固定长度
//...
```
**左、右、居中对齐**
```cpp
std::string str1 = std::format("|{:*<10}|", "left"); //|left******|
std::string str2 = std::format("|{:_>10}|", "right"); //|_____right|
std::string str3 = std::format("|{:-^10}|", "center"); //|--center--|
std::string str4 = std::format("|{:0<5}|", 11); //|11000|
```
#### 2.1.3 STL容器（C++23开始支持）
参数支持使用容器，比如
```cpp
std::vector<int> v = {1, 2, 3};
std::string str2 = std::format("the vector is {}", v);
//the vector is [1, 2, 3]
```
### 2.2 format_to format_to_n
相比format，不需要再额外传值返回，而是直接写到缓冲区中，也可以设置缓冲区大小，format_to_n 还可以设置最大写入大小。总的来说，需要传入的都是迭代器，而不是原字符串

**输出到空串**
```cpp
s1.resize(90);
auto it = std::format_to(s1.begin(), "Hello {}! The answer is {}", "World", 42);
s1.resize(it - s1.begin(), 0);
```
这种方式适合空 string 作为缓冲区的情况，传入起始迭代器，格式化输出到 string 完成后，会返回最后末尾迭代器，两者相减就可以获得格式化输出的长度。

**输出到非空字符串**
```cpp
s2 = "Yelling: ";
auto backIt = std::back_inserter(s2);
std::format_to(backIt, "Hello {}! Answer is{}", "World", 42);
std::cout << s2 << std::endl;
```
这种方式适合未知长度字符串作为缓冲区的情况。因为不知道长度，还需要后面插入自动迭代，所以适合传入 `back_inserter` 获取的迭代器。这种方式不需要对字符串未输入的部分作格式化

**输出到输出流**
```cpp
std::format_to(std::ostreambuf_iterator<char>(std::cout), "Hello {}! Answer is {}\n", "World", 42);
```
这种方式需要传入输出流的迭代器，之后输出的内容就是直接输出到标准输出
#### 2.2.1 back_inserter
`back_inserter`返回的迭代器在每次解引用时，都等价于调用 `push_back`，并且会自动迭代器到新的末尾迭代器位置
```cpp
std::vector<int> v;
auto tmpIt = std::back_inserter(v);
*tmpIt = 1;
*tmpIt = 2;
*tmpIt = 3;
for (auto e: v) {
	std::cout << e << " ";
}
std::cout << std::endl;
```
## 2.3 vformat
这里的 v，是 view，视图的意思。使用上和 format 完全一样，但是第一个参数需要传入一个视图，后两个参数也需要特殊的 format_args（通过 make_format_args构造）。
vformat 并不会直接在传入视图上作出格式化更改（也不能），而是会直接构建一个新的字符串返回，减少了在格式化原有字符串时字符串移动等的开销

**使用样例1：日志器**
```cpp
class log {
public:
    enum class Level{
        error,
        debug
    };

    template <class...Args>
    void debug(Level level, std::string_view sv, Args&&...args) {
        auto format_args = std::make_format_args(args...);
        auto buff = std::vformat(sv, format_args);
        outputMessage(Level::debug, buff);
    }

    template <class...Args>
    void error(Level level, std::string_view sv ,Args&&...args) {
        auto format_args = std::make_format_args(args...);
        auto buf = std::vformat(sv, format_args);
        outputMessage(Level::error, buf);
    }
private:
    void outputMessage(Level level, const std::string& buffer) {
        if (level == Level::debug) {
            std::cout << "Debug: " << buffer << std::endl;
        }
        else {
            std::cout << "Error: " << buffer << std::endl;
        }
    }
};
```

**使用样例2：延迟格式化**
```cpp
class delayFormat {
public:
    template <class...Args>
    void addFormat(std::string_view sv, Args&&...args) {
        auto newFormat = [=]() {
            return std::vformat(sv, std::make_format_args(args...));
        };
        _formatters.emplace_back(newFormat);
    }

    void executeAll() {
        for (auto format: _formatters) {
            auto ret = format();
            std::cout << ret << std::endl;
        }
    }
private:
    std::vector<std::function<std::string()>> _formatters;
};
```
## 2.4 formatter
formatter 允许自定义实现 format，需要实现两个部分：parse 和 format
* `parse`，在编译时调用，解析格式化参数
* `format`，在运行时调用，进行格式化处理

**formatter，偏特化的语法格式**
```cpp
template<>
struct formatter<typename>{
	constexpr auto parse(std::format_parse_context& cxt){
		//...
		return cxt.begin();
	}
	
	auto format(const typename& type, std::format_context& cxt){
		//...
		return std::format_to(cxt.out(), "format{}", type.member);
	}
}
```
比如下面对 Person 类的 formatter 特化
```cpp
struct Person {
    const std::string name;
    const int age;
    const std::string gender;
};

template<>
class std::formatter<Person> {
public:
    char option = 's';
    constexpr auto parse(std::format_parse_context& cxt) {
        auto it = cxt.begin();
        char ch = *it;
        switch (ch) {
            case 's':
            case 'l':
            case 'j':
                option = ch;
                it++;
                break;
            default:
                throw std::format_error("expected one of j, l, s");
                break;
        }
        if (it != cxt.end() && *it != '}') {
            throw std::format_error("expected one argument or should ended with }");
        }
        return it;
    }

    auto format(const Person& p, std::format_context& cxt) const {
        if (option == 'j') {
            return std::format_to(cxt.out(), R"({{"name":"{}","age":{},"gender":"{}"}})", p.name, p.age, p.gender); //注意，R"()"，只能够正常在字符串中使用引号，但是花括号不受影响，只能通过{{}}转义花括号
        }
        else if (option == 'l') {
            return std::format_to(cxt.out(), "name: {}, age: {}, gender: {}", p.name, p.age, p.gender);
        }
        else if (option == 's') {
            return std::format_to(cxt.out(), "name: {}", p.name);
        }
        else {
            throw std::format_error("Unknown format option");
        }
    }
};
```

在 C++23 中，可以实现对 STL 容器的格式化输出。同时支持 `range_formatter` 对范围容器的格式化，可以极大简化对符合范围的容器的格式化。
使用 range_formatter 的 parse、format、set_separator、set_brackets，就可以方便的自定义范围容器的格式化
```cpp
template<class T>
struct std::formatter<std::vector<T>> {
public:
    std::range_formatter<T, char> rf;
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = rf.parse(ctx);
        rf.set_separator("----");
        rf.set_brackets("|", "|"); //注意这里要传字符串，字符无法转化为 basic_string_view
        return it;
    }

    auto format(const std::vector<T>& v, std::format_context& ctx)const  {
        return rf.format(v, ctx);
    }
};
```