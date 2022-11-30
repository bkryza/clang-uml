#include <memory>
#include <string>

namespace clanguml {
namespace t20009 {
template <typename T> struct A {
    void a(T arg) { }
};

template <typename T> struct B {
    void b(T arg) { a->a(arg); }

    std::unique_ptr<A<T>> a;
};

using BFloatPtr = std::shared_ptr<B<float>>;

void tmain()
{
    std::shared_ptr<B<std::string>> bstring;
    auto bint = std::make_unique<B<int>>();
    BFloatPtr bfloat;

    bstring->b("b");
    bint.get()->b(42);
    bfloat->b(1.0);
}
}
}