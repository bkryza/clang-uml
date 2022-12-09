namespace clanguml {
namespace t20016 {
struct A {
    void a1(int a) { }
    template <typename T> T a2(const T &a) { return a; }
};

template <typename T> struct B {
    void b1(T b) { a_.a1(1); }

    template <typename F> F b2(T t) { return a_.a2(t); }

    A a_;
};

void tmain()
{
    B<long> b;

    b.b1(1);

    b.b2<int>(2);
}
}
}