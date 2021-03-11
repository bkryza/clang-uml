namespace external {
class C {
};
}

namespace clanguml {
namespace t00011 {

class B;

class A {
private:
    void foo() {}
    friend class B;
    friend class external::C;
};

class B {
public:
    void foo() { m_a->foo(); }
    A *m_a;
};
}
}
