namespace clanguml {
namespace t20005 {

template <typename T> struct A {
    T a(T arg) { return arg; }
};

template <typename T> struct B {
    T b(T arg) { return a_.a(arg); }

    A<T> a_;
};

template <typename T> struct C {
    T c(T arg) { return b_.b(arg); }

    B<T> b_;
};

}
}