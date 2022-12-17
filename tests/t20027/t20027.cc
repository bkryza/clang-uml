namespace clanguml {
namespace t20027 {

class A {
public:
    void a() { aa(); }

protected:
    void aa() { aaa(); }

private:
    void aaa() { }
};

void tmain()
{
    A a;

    a.a();
}
}
}