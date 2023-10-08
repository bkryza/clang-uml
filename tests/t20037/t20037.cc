namespace clanguml {
namespace t20037 {

struct A {
    A()
        : a{100}
    {
    }

    int a;
};

struct B {
    int get() { return b; }

    int b{100};
};

B initb() { return B{}; }

int c() { return 1; }

int a()
{
    static A a;
    static B b = initb();

    return a.a + b.get() + c();
}

void tmain(int argc, char **argv)
{
    auto a1 = a();
    auto a2 = a();
    auto a3 = a();
}
}
}