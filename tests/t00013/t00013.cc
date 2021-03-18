#include <algorithm>
#include <map>
#include <numeric>
#include <string>
#include <variant>

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

class R {
public:
    int get_a(A *a) { return a->a; }
    int get_b(B &b) { return b.b; }
    // TODO: int get_b(const B &b) { return b.b; }
    int get_c(C c) { return c.c; }
    int get_d(D &&d) { return d.d; }
};
}
}
