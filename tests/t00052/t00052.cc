namespace clanguml {
namespace t00052 {

class A {
public:
    template <typename T> T a(T p) { return p; }

    template <typename F, typename Q> void aa(F &&f, Q q) { f(q); }
};

template <typename T> class B {
public:
    T b(T t) { return t; }

    template <typename F> T bb(F &&f, T t) { return f(t); }
};

}
}