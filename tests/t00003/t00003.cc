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

    int public_member;

protected:
    void protected_method() {}

    int protected_member;

private:
    void private_method() {}

    int private_member;

    int a, b, c;
};
}
}
