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
