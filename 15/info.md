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