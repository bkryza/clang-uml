#include <iostream>
#include <string>

namespace clanguml {
namespace t20041 {

template <typename... Args> struct A;

template <typename First, typename... Rest> struct A<First, Rest...> {
    void print(First first, Rest... rest)
    {
        std::cout << first << std::endl;

        A<Rest...> a;
        a.print(rest...);
    }
};

template <> struct A<> {
    void print() { }
};

void tmain()
{
    using namespace std::literals::string_literals;

    A<int, double, std::string> a;
    a.print(1, 3.14, "test"s);
}
}
}