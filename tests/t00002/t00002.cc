#include <vector>

namespace clanguml {
namespace t00002 {

class A {
public:
    virtual void foo() {}
};

class B : public A {
};

class C : public A {
};

class D : public B, public C {
    std::vector<A *> as;
};
}
}
