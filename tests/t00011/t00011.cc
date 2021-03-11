namespace clanguml {
namespace t00011 {

class B;

class A {
private:
    void foo() {}
    friend class B;
};

class B {
public:
    void foo() { m_a->foo(); }
    A *m_a;
};
}
}
