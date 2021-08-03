#include <memory>
#include <vector>

namespace clanguml {
namespace t00032 {

struct Base {
};

struct TBase {
};

struct A {
    void operator()() { }
};

struct B {
    void operator()() { }
};

struct C {
    void operator()() { }
};

template <typename T, typename... Ts>
struct Overload : public Base, public T, public Ts... {
    using Ts::operator()...;
};

template <class... Ts> Overload(Ts...) -> Overload<Ts...>;

struct R {
    Overload<TBase, A, B, C> overload;
};

} // namespace t00032
} // namespace clanguml
