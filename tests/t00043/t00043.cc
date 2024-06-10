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

struct EE {
    void ee(E *e) { }
};

struct EEE {
    void eee(EE *e) { }
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

struct II;
struct III {
    void iii(II *i) { }
};

struct II {
    void ii() { }
};

struct J {
    void i(I *i) { }
    void ii(II *ii) { }
};

} // namespace dependencies
} // namespace clanguml::t00043
