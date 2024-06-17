namespace clanguml {
namespace t00076 {

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

struct A { };

struct B : public A {
    F *f;
    Color c;
    G g;
    /// @uml{composition[0..1:1..*]}
    J j;

    struct BB { };

    BB *bb;

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

struct F { };

struct I {
    void i(B *b) { (void)b; }
};
}
}