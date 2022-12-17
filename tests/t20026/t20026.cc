namespace clanguml {
namespace t20026 {

struct A {
    virtual void a() { }
};

struct B : public A {
    void a() override { }
};

struct C : public B {
    void a() override { }
};

int tmain()
{
    C *a{};

    dynamic_cast<A *>(a)->a();

    return 0;
}
}
}