#include <vector>

namespace clanguml {
namespace t20021 {
struct A {
    int a1() { return 0; }
    int a2() { return 1; }
    int a3() { return 2; }
};

struct B {
    void log() { }

    int b1() const { return 3; }
    int b2() const { return 4; }
};

struct C {
    int c1() { return 1; }
    int c2() { return 2; }
    int c3() { return 3; }
    int c4() { return c5(); }
    int c5() { return 5; }

    std::vector<int> &contents() { return contents_; }

    std::vector<int> contents_;
};

int tmain()
{
    A a;
    std::vector<B> b;
    C c;

    int i = 10;
    while (i -= c.c4()) {
        int j = a.a3();
        do {
            for (int l = a.a2(); l > c.c1(); l -= c.c2())
                a.a1();
        } while (j -= c.c3());
    }

    int result = 0;
    for (const auto &bi : b) {
        result += bi.b2();
    }

    for (const auto &ci : c.contents()) {
        result += ci;
    }

    return b.front().b2() + result;
}
}
}