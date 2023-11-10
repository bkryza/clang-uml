#include <memory>
#include <vector>

namespace clanguml {
namespace t00068 {

struct B { };

struct BB {
    std::vector<B> b;
};

enum class AKind { OneA, TwoA, ThreeA };

struct A { };

struct AA : public A { };

struct AAA : public AA {
    BB *bb;
    AKind akind;
};

struct R {
    AAA *aaa;
};

struct RR {
    std::shared_ptr<R> r;
};
}
}