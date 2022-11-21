#include <string>

namespace clanguml {
namespace t20006 {

template <typename T> struct A {
    T a(T arg) { return arg; }
    T a1(T arg) { return arg; }
};

template <typename T> struct B {
    T b(T arg) { return a_.a(arg); }
    A<T> a_;
};

template <> struct B<std::string> {
    std::string b(std::string arg) { return arg; }
    A<std::string> a_;
};

void tmain()
{
    B<int> bint;
    B<std::string> bstring;

    bint.b(1);
    bstring.b("bstring");
}
}
}