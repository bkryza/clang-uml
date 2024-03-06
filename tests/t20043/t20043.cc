namespace clanguml {
namespace t20043 {
struct A {
    int a() { return 1; }
};

struct B {
    A a;
    int b() { return a.a(); }
};

struct C {
    B b;
    int c() { return b.b(); }
};

namespace detail {
struct E {
    void e() { }
};
} // namespace detail

struct D {
    C c;
    detail::E e;
    int d()
    {
        e.e();
        return c.c();
    }
};

struct F {
    void f() { }
};

int tmain()
{
    D d;
    F f;

    f.f();

    return d.d();
}
}
}