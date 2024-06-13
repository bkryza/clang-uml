namespace clanguml {
namespace t00077 {

enum Color { red, green, blue };

struct F;
struct GG { };
struct G {
    GG gg;
};
struct GGG {
    G g;
};
struct H { };
struct J { };

struct Base { };

struct A : public Base { };

struct B : public A {
    F *f;
    Color c;
    G g;
    /// @uml{composition[0..1:1..*]}
    J j;

    void a(H *h) { (void)h; }
};

struct C : public B { };

struct D : public C { };

struct EE { };

struct E {
    B *b;
    EE *ee;
};

struct EEE {
    E *e;
};

struct FF { };
struct F {
    FF *ff;
};
struct FFF {
    F *f;
};

struct I {
    void i(B *b) { (void)b; }
};

struct KKK { };

struct K {
    B b;
    KKK kkk;
};

struct KK {
    K k;
};
}
}