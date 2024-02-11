#include <iostream>
#include <string>

namespace clanguml {
namespace t20040 {
void print() { }

template <typename T, typename... Ts> void print(T head, Ts... tail)
{
    std::cout << head << std::endl;
    print(tail...);
}

template <typename... Ts> void doublePrint(Ts... args)
{
    print(args + args...);
}

void tmain()
{
    using namespace std::literals::string_literals;

    print(1, 3.14, "test"s);

    doublePrint("test"s, 2024 / 2);
}
}
}