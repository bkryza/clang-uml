namespace clanguml {
namespace t20056 {
struct A {
    void a() { aa(); }

    void aa() { aaa(); }

    void aaa() { }
};

struct B {
    void b() { bb(); }

    void bb() { bbb(); }

    void bbb() { a.a(); }

    A a;
};

struct C {
    void c() { cc(); }

    void cc() { ccc(); }

    void ccc() { b.b(); }

    B b;
};

void tmain()
{
    A a;
    B b;
    C c;

    c.c();
    c.c();
    c.c();

    b.b();

    a.a();
}
}
}
