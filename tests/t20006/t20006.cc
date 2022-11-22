#include <string>

namespace clanguml {
namespace t20006 {

template <typename T> struct A {
    T a_int(T arg) { return arg + 1; }
    T a_string(T arg) { return arg + "_string"; }
};

template <typename T> struct B {
    T b(T arg) { return a_.a_int(arg); }
    A<T> a_;
};

template <> struct B<std::string> {
    std::string b(std::string arg) { return a_.a_string(arg); }
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