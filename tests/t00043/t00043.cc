namespace clanguml::t00043 {

namespace dependants {
struct A { };

struct B {
    void b(A *a) { }
};

struct BB {
    void bb(A *a) { }
};

struct C {
    void c(B *b) { }
};

struct D {
    void d(C *c) { }
    void dd(BB *bb) { }
};

struct E {
    void e(D *d) { }
};

struct F { };
} // namespace dependants

namespace dependencies {

struct G { };

struct GG { };

struct H {
    void h(G *g) { }
    void hh(GG *gg) { }
};

struct HH {
    void hh(G *g) { }
};

struct I {
    void i(H *h) { }
};

struct J {
    void i(I *i) { }
};

} // namespace dependencies
} // namespace clanguml::t00043
