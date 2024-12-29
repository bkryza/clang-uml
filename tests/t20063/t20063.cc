class t00089_A {
public:
    void A() { }
};

namespace thirdparty {
namespace lib1 {
void a_impl() { }
class A {
public:
    void a() { a_impl(); }
};
}
}

namespace clanguml {
namespace t20063 {
namespace thirdparty {
namespace lib1 {
void a_impl() { }
class A {
public:
    void a() { a_impl(); }
};
}
namespace lib3 {
class C {
public:
    void c() { }
};
}
}

void tmain()
{
    t00089_A A;
    A.A();

    ::thirdparty::lib1::A root_a;
    root_a.a();

    thirdparty::lib1::A a;
    a.a();

    thirdparty::lib3::C c;
    c.c();
}
}
}