#include <cstdint>

namespace clanguml {
namespace t20034 {
struct A {
    void a1() { }
    void a2() { }
    void a3() { }
};

struct C;

struct B {
    void b1()
    {
        a.a1();
        a.a2();
    }
    void b2() { a.a2(); }
    void b3() { a.a3(); }
    void b4();

    A a;

    C *c;
};

struct C {
    void c1() { b.b1(); }
    void c2() { b.b2(); }
    void c3()
    {
        if (reinterpret_cast<uint64_t>(&b) == 0xbadc0de)
            c3();
        else
            c2();
    }

    void c4() { b.b4(); }

    B b;
};

struct D {
    void d1() { c.c1(); }
    void d2()
    {
        c.c1();
        a.a2();
        c.c2();
        c.c3();
        a.a2();

        c.c4();

        auto l = [this]() { a.a2(); };
        l();
    }
    void d3() { c.c3(); }

    A a;
    C c;
};

void B::b4()
{
    c->c4();
    b2();
}
}
}