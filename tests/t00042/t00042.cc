#include <string>

namespace clanguml::t00042 {

template <typename T> struct A {
    T a;
};

template <> struct A<void> {
    void *a;
};

template <typename T, typename K> struct B {
    T b;
    K bb;
};

template <typename T> struct C {
    T c;
};

struct R {
    A<double> a_double;
    A<std::string> a_string;

    B<int, float> b_int_float;

    C<int> c_int;
};

} // namespace clanguml::t00042
