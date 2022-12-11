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

int tmain()
{
    A a;
    std::vector<B> b;

    int i = 10;
    while (i--) {
        int j = a.a3();
        do {
            for (int l = a.a2(); l > 0; l--)
                a.a1();
        } while (j--);
    }

    int result = 0;
    for (const auto &bi : b) {
        result += bi.b2();
    }

    return b.front().b2() + result;
}
}
}