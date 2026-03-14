#include <iostream>

constexpr int fib(int a)
{
    return a == 1 ? 1 : a + fib(a - 1);
}

class Date
{
public:
    constexpr Date(int year, int month, int day)
        : _year(year), _month(month), _day(day){
            if(month > 12)
            {
                _year = _month = _day = 0;
            }
        }
    constexpr int getYear() const{
        return _year;
    }
private:
    int _year; int _month; int _day;
};

struct s
{
    constexpr s(int a, int b)
        : a(a), b(b){}
    int a, b;
};
constexpr s func()
{
    return s(1, 1);
}

template <class T>
constexpr int add(T t1, T t2){
    return t1 + t2;
}

template <class T>
constexpr auto get_value(T t)
{
    if constexpr(std::is_pointer_v<T>){
        return *t;
    }
    else
    {
        return t;
    }
}

constexpr void dynamic_src()
{
    int* pa = new int{42};
    delete pa;
}

constexpr int devide(int a, int b)
{
    try
    {
        if(b == 0) throw(std::logic_error("devided by zero"));
        else return a / b;
    }
    catch(...)
    {
        return -1;
    }
}

int main()
{
    constexpr int ret = fib(100);
    constexpr Date d(1, 2, 3);
    std::cout << d.getYear();
    constexpr int ret1 = add<int>(1, 2);

    const int n = 10;
    auto lambda_func = [n]()constexpr {
        int a = n + 10; return a;
    };
    
    int val = 10;
    int* pval = &val;
    get_value<int*>(pval);

    dynamic_src();
    int a = devide(1, 1);
    std::cout << a;
    return 0;
}