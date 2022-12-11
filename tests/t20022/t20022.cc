#include <memory>

namespace clanguml {
namespace t20022 {
class B;

class A {
public:
    A(std::unique_ptr<B> b);

    void a();

    std::unique_ptr<B> b_;
};

class B {
public:
    void b() { }
};

A::A(std::unique_ptr<B> b)
    : b_{std::move(b)}
{
}

void A::a() { b_->b(); }

int tmain()
{
    A a{std::make_unique<B>()};

    a.a();

    return 0;
}
}
}