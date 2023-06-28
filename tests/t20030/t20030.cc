namespace clanguml {
namespace t20030 {

int magic() { return 42; }

class A {
public:
    A() { create(); }

    A(int v) { a_ = v; }

    A &operator=(const A &a)
    {
        set(a.a_);
        return *this;
    }

    A &operator+=(int a)
    {
        add(a);
        return *this;
    }

    int value() const { return a_; }

private:
    void create() { a_ = 0; }

    void add(int a) { a_ += a; }
    void set(int a) { a_ = a; }

    int a_;
};

void tmain(int a)
{
    A an_a{magic()};
    an_a += 1;
}

int tmain(bool f, int a)
{
    auto an_a = A();
    auto an_b = A();
    an_a += 2;
    an_b = an_a;
    return an_b.value();
};

}
}