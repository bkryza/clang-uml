namespace clanguml {
namespace t20045 {

template <typename F> int a1(F &&f) { return f(42); }

int a2(int x) { return 2; }

int a3(int x) { return 3; }

struct B {
    int b1(int x) { return x + 1; }
    int b2(int x) { return x + 2; }
};

class C {
public:
    explicit C(int x)
        : x_{x}
    {
    }

    int get_x() const { return x_; }

private:
    int x_;
};

int tmain()
{
    B b;

    // \uml{call clanguml::t20045::a2(int)}
    auto v1 = a1(a2);

    auto v2 = a1([](auto &&arg) { return a3(arg); });

    auto v3 = a1([&](auto &&arg) { return b.b1(arg); });

    auto v4 = a1([](auto &&arg) {
        C c(arg);
        return c.get_x();
    });

    return 0;
}
}
}