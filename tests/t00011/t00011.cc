namespace external {
class C { };
}

namespace clanguml {
namespace t00011 {

class B;

template <typename T> class D {
    T value;
};

class A {
public:
    void foo() { }
    friend class B;
    friend class external::C;
    // TODO
    template <typename T> friend class D;
    // TODO
    friend class D<int>;
    friend class D<A>;
};

class B {
public:
    void foo() { m_a->foo(); }
    A *m_a;
};
} // namespace t00011
} // namespace clanguml
