#include <array>
#include <map>
#include <vector>

namespace clanguml {
namespace t00006 {
class A { };

class B { };

class C { };

class D { };

class E { };

class F { };

class G { };

class H { };

class I { };

class J { };

class K { };

class L { };

class M { };

class N { };

class NN { };

class NNN { };

template <typename T> class custom_container {
public:
    std::vector<T> data;
};

class R {
public:
    std::vector<A> a;
    std::vector<B *> b;

    std::map<int, C> c;
    std::map<int, D *> d;

    custom_container<E> e;

    std::vector<std::vector<F>> f;
    std::map<int, std::vector<G *>> g;

    std::array<H, 10> h;
    std::array<I *, 5> i;

    J j[10];
    K *k[20];

    std::vector<std::pair<L, M>> lm;

    std::tuple<N, NN, NNN> ns;
};
} // namespace t00006
} // namespace clanguml
