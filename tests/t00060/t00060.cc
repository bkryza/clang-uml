namespace clanguml {
namespace t00060 {
struct A { };
struct B : public A { };
struct C : public A { };
struct D : public B, public C { };
struct E : public C { };
struct F : public D { };

template <typename T> struct G {
    T g;
};

template <typename T, typename P> struct H : public G<T> {
    G<T> h;
    P hh;
};

}
}