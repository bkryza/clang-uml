#include <cmath>
#include <cstdint>

namespace clanguml {
namespace t20020 {
struct A {
    int a1() { return 0; }
    int a2() { return 1; }
    int a3() { return 2; }
    int a4() { return 3; }
    int a5() { return 4; }
};

struct B {
    void log() { }

    int b1() { return 3; }
    int b2() { return 4; }
};

struct C {
    void log() const { }

    void c1() const
    {
        if (c2())
            log();
    }

    bool c2() const { return true; }

    int c3(int x) { return x * 2; }
};

template <typename T> struct D {

    T d1(T x, T y) { return x + y; }
};

int tmain()
{
    A a;
    B b;
    C c;
    D<int> d;

    int result{0};

    if (reinterpret_cast<uint64_t>(&a) % 100 == 0ULL) {
        result = a.a1();
    }
    else if (reinterpret_cast<uint64_t>(&a) % 100 == 42ULL) {
        result = a.a5();
    }
    else if (reinterpret_cast<uint64_t>(&a) % 64 == 0ULL) {
        if (c.c3(a.a2()) > 2)
            result = b.b1();
        else if (a.a3() % 2)
            result = b.b2();
        else
            result = 0;
    }
    else {
        result = a.a4();
    }

    b.log();

    if (true)
        c.c1();

    if (true)
        d.d1(1, 1);

    // This if/else should not be included in the diagram at all
    // as the calls to std will be excluded by the diagram filters
    if (result != 2) {
        result = std::exp(result);
    }
    else if (result == 3) {
        result = 4;
    }
    else {
        result = std::exp(result + 1);
    }

    return result;
}
}
}