#include <memory>

namespace clanguml {
namespace t00022 {

class A {
public:
    void template_method()
    {
        method1();
        method2();
    }

protected:
    virtual void method1() = 0;
    virtual void method2() = 0;
};

class A1 : public A {
protected:
    void method1() override { }
    void method2() override { }
};

class A2 : public A {
protected:
    void method1() override { }
    void method2() override { }
};
} // namespace t00022
} // namespace clanguml
