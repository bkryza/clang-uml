namespace clanguml {
namespace t00076 {

enum Color { red, green, blue };

struct F;
struct G { };
struct H { };
struct J { };

struct A { };

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

struct E {
    B *b;
};
struct F { };

struct I {
    void i(B *b) { (void)b; }
};
}
}