#include <functional>

namespace clanguml {
namespace t20031 {
int magic() { return 42; }
int zero() { return 0; }
int one() { return 1; }
int execute(std::function<int()> f) { return f(); }

class A {
public:
    A() { create(); }

    A(int v) { a_ = v; }

    A &operator=(const A &a)
    {
        set(a.a_);
        return *this;
    }

    A &operator+=(int a)
    {
        add(a);
        return *this;
    }

    int value() const { return a_; }

private:
    void create() { a_ = 0; }

    void add(int a) { a_ += a; }
    void set(int a) { a_ = a; }

    int a_;
};

void tmain(int a)
{
    A an_a{magic()};
    an_a += 1;
}

int tmain(bool f, int a)
{
    auto generate_zero = []() { return zero(); };
    auto an_a = A();
    auto an_b = A();

    an_a += generate_zero();

    // @todo #168
    an_a += execute([]() { return one(); });

    an_b = an_a;

    return an_b.value();
};

}
}