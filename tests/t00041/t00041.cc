namespace clanguml::t00041 {

struct B { };

struct A { };

class AA : public A { };

struct R { };

struct RR;

struct D {
    RR *rr;
};

struct E { };

struct F { };

namespace detail {
struct G { };
} // namespace detail

struct H { };

struct RR : public R {
    E *e;
    F *f;
    detail::G *g;

    enum K { One, Two, Three };

    K k;

    void foo(H *h) { }
};

struct RRR : public RR { };

namespace ns1 {
struct N { };

struct NN : public N { };

struct NM : public N { };
}

enum class Color { Red, Green, Blue };

struct S {
    Color c;
};

struct T {
    enum class Direction { Left, Right };
    Direction d;
};

} // namespace clanguml::t00041
