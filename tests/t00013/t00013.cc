#include <algorithm>
#include <map>
#include <numeric>
#include <string>
#include <variant>

namespace ABCD {
template <typename T> struct F {
    T f;
};
}
namespace clanguml {
namespace t00013 {

struct A {
    int a;
};

struct B {
    int b;
};

struct C {
    int c;
};

class R;

struct D {
    int d;
    void print(R *r) {}
};

template <typename T> struct E {
    T e;
};

using namespace ABCD;
class R {
public:
    int get_a(A *a) { return a->a; }
    int get_b(B &b) { return b.b; }
    // TODO: int get_const_b(const B &b) { return b.b; }
    int get_c(C c) { return c.c; }
    int get_d(D &&d) { return d.d; }
    // Dependency relationship should be rendered only once
    int get_d2(D &&d) { return d.d; }

    template <typename T> T get_e(E<T> &e) { return e.e; }
    int get_int_e(E<int> &e) { return e.e; }

    template <typename T> T get_f(const F<T> &f) { return f.f; }
};
}
}
