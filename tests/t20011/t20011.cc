namespace clanguml {
namespace t20011 {

struct A {
    void a(int i = 10)
    {
        if (i > 0)
            a(i - 1);
    }

    void b(int i = 10) { c(i); }
    void c(int i) { d(i); }
    void d(int i)
    {
        if (i > 0)
            b(i - 1);
        else
            a();
    }
};

void tmain()
{
    A a;

    a.a();

    a.b();
}
}
}