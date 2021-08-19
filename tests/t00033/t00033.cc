#include <memory>
#include <vector>

namespace clanguml {
namespace t00033 {

template <typename T> struct A {
    T aaa;
};

template <typename T> struct B {
    T bbb;
};

template <typename T> struct C {
    T ccc;
};

struct D {
    int ddd;
};

struct R {
    A<B<std::unique_ptr<C<D>>>> abc;
};

} // namespace t00033
} // namespace clanguml
