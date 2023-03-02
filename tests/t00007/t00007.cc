#include <memory>

namespace clanguml {
namespace t00007 {
class A { };

class B { };

class C { };

class R {
public:
    std::unique_ptr<A> a;
    std::shared_ptr<B> b;
    std::weak_ptr<C> c;
};
} // namespace t00007
} // namespace clanguml
