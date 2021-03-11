namespace external {
class C {
};
}

namespace clanguml {
namespace t00011 {

class B;

template <typename T> class D {
    T value;
};

class A {
private:
    void foo() {}
    friend class B;
    friend class external::C;
    template <typename T> friend class D;
    // TODO: Add friend for a template specialization
};

class B {
public:
    void foo() { m_a->foo(); }
    A *m_a;
};
}
}
