#include <memory>
#include <vector>

namespace clanguml {
namespace t00030 {

class A { };

class B { };

class C { };

class D { };

class E { };

struct R {
    /// @uml{association[]}
    A aaa;

    /// @uml{composition[0..1:1..*]}
    std::vector<B> bbb;

    /// @uml{aggregation[0..1:1..5]}
    std::vector<C> ccc;

    /// @uml{association[:1]}
    D ddd;

    /// @uml{aggregation[:1]}
    E *eee;
};

} // namespace t00030
} // namespace clanguml
