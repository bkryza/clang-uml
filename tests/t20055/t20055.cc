namespace clanguml {
namespace t20055 {
namespace ns1 {

void d() { }

struct A {
    void a() { }
};

struct B {
    A a;
    void b()
    {
        a.a();
        d();
    }
};

} // namespace ns1
namespace ns2 {
void f() { }
struct C {
    ns1::B b;
    void c()
    {
        b.b();
        f();
    }
};

void tmain()
{
    C c;
    c.c();
}

} // namespace ns2
}
}