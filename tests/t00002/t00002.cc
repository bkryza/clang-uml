#include <vector>

namespace clanguml {
namespace t00002 {

class A {
public:
    virtual void foo_a() = 0;
    virtual void foo_c() = 0;
};

class B : public A {
public:
    virtual void foo_a() override { }
};

class C : public A {
public:
    virtual void foo_c() override { }
};

class D : public B, public C {
public:
    void foo_a() override
    {
        for (auto a : as)
            a->foo_a();
    }

    void foo_c() override
    {
        for (auto a : as)
            a->foo_c();
    }

private:
    std::vector<A *> as;
};

//
// NOTE: libclang fails on:
//
//   class D : public virtual B, public virtual C {
//
class E : virtual public B, virtual public C {
public:
    void foo_a() override
    {
        for (auto a : as)
            a->foo_a();
    }

    void foo_c() override
    {
        for (auto a : as)
            a->foo_c();
    }

private:
    std::vector<A *> as;
};
}
}
