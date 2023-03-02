#include <string>

namespace clanguml {
namespace t20006 {

template <typename T> struct A {
    T a1(T arg) { return arg; }
    T a2(T arg) { return arg + arg; }
};

template <typename T> struct B {
    T b(T arg) { return a_.a1(arg); }
    A<T> a_;
};

template <> struct B<std::string> {
    std::string b(std::string arg) { return a_.a2(arg); }
    A<std::string> a_;
};

template <typename T> struct AA {
    void aa1(T t) { }
    void aa2(T t) { }
};

template <typename T, typename F> struct BB {
    void bb1(T t, F f) { aa_.aa1(t); }
    void bb2(T t, F f) { aa_.aa2(t); }

    AA<T> aa_;
};

template <typename T> struct BB<T, std::string> {
    void bb1(T t, std::string f) { aa_->aa2(t); }
    void bb2(T t, std::string f) { aa_->aa1(t); }

    BB(AA<T> *aa)
        : aa_{aa}
    {
    }

    AA<T> *aa_;
};

template <typename T> struct BB<T, float> {
    void bb1(T t, float f) { bb2(t, f); }
    void bb2(T t, float f) { aa_.aa2(t); }

    BB(AA<T> &aa)
        : aa_{aa}
    {
    }

    AA<T> &aa_;
};

void tmain()
{
    B<int> bint;
    B<std::string> bstring;

    bint.b(1);
    bstring.b("bstring");

    BB<int, int> bbint;
    AA<int> aaint;
    BB<int, std::string> bbstring{&aaint};
    BB<int, float> bbfloat{aaint};

    bbint.bb1(1, 1);
    bbint.bb2(2, 2);

    bbstring.bb1(1, "calling aa2");
    bbstring.bb2(1, "calling aa1");

    bbfloat.bb1(1, 1.0f);
}
}
}