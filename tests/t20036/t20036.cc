#include <cstdint>

namespace clanguml {
namespace t20036 {
struct A {
    void a1() { }
    void a2() { }
    void a3() { }
};

struct B {
    void b1() { a.a2(); }
    void b2() { a.a2(); }
    void b3() { a.a3(); }

    A a;
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

    void c4() { b.b2(); }

    B b;
};

struct D {
    void d1() { c.c2(); }
    void d2() { c.c2(); }
    void d3() { a.a2(); }

    A a;
    C c;
};
}
}