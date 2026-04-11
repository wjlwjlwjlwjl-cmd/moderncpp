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

![coroutine](C:\GitFiles\moderncpp\20\coroutine.PNG)
