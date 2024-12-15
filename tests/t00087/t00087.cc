namespace clanguml {
namespace t00087 {

class FooClass {
    int foo_;

    void *pImpl_;

public:
    FooClass() { }

    int getFoo() const { return foo_; }

    void setFoo(int f) { foo_ = f; }

    void foo() { }

    void bar() { }

    static void makeFooClass_static() { }

    static int fooCount_static;
};

enum FooEnum {};

template <typename T> class Bar {
    T bar;
};
}
}