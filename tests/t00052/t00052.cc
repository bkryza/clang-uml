#include <cmath>

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

template <typename T> class C {
    template <typename P> T c(P p) { return static_cast<T>(p); }
};

template <> template <> int C<int>::c<double>(double p)
{
    return std::floor(p);
}

struct R {
    A a;
    B<int> b;
    C<int> c;
};
}
}