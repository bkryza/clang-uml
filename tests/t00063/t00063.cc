namespace clanguml {
namespace t00063 {
class A { };

enum B { b1, b2, b3 };

enum class C { c1, c2, c3 };

typedef enum { d1, d2, d3 } D;

struct R {
    enum RR { r1, r2, r3 };
    typedef enum { rr1, rr2, rr3 } RRR;
    A a;
    D d;
};
}
}