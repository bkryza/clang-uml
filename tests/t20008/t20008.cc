#include <string>
#include <type_traits>

namespace clanguml {
namespace t20008 {

template <typename T> struct A {
    void a1(T arg) { }
    void a2(T arg) { }
    void a3(T arg) { }
};

template <typename T> struct B {
    A<T> a;

    void b(T arg)
    {
        if constexpr (std::is_integral_v<T>) {
            a.a1(arg);
        }
        else if constexpr (std::is_pointer_v<T>) {
            a.a2(arg);
        }
        else {
            a.a3(arg);
        }
    }
};

void tmain()
{
    using namespace std::string_literals;

    B<int> bint;
    B<const char *> bcharp;
    B<std::string> bstring;

    bint.b(1);
    bcharp.b("1");
    bstring.b("1"s);
}
}
}