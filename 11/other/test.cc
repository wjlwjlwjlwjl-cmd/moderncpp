#include <iostream>

constexpr int fib(int a)
{
    return a == 1 ? 1 : a + fib(a - 1);
}

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

int main()
{
    constexpr int ret = fib(100);
    constexpr Date d(1, 2, 3);
    std::cout << d.getYear();
    constexpr int ret1 = add<int>(1, 2);
    return 0;
}