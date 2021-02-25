#include <functional>

namespace clanguml {
namespace t00003 {

class A {
public:
    A() = default;
    A(A &&) = default;
    A(const A &) = default;
    virtual ~A() = default;

    void basic_method() {}
    static int static_method() { return 0; }
    void const_method() const {}
    auto auto_method() { return 1; }

    int public_member;
    static int static_int;
    static const int static_const_int = 1;

protected:
    void protected_method() {}

    int protected_member;

    std::function<bool(const int)> compare = [this](const int v) {
        return private_member > v;
    };

private:
    void private_method() {}

    int private_member;
    int a, b, c;
};

int A::static_int = 1;
}
}
