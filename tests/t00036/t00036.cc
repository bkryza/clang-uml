namespace clanguml {
namespace t00036 {

namespace ns1 {

enum class E { blue, yellow };

namespace ns11 {

template <typename T> struct A {
    T a;
};

namespace ns111 {

struct B {
    A<int> a_int;
};

}
}
}

namespace ns2 {
namespace ns22 {

// TODO: Fix for incomplete struct C declaration "struct C;"
struct C {
};

}
}

} // namespace t00036
} // namespace clanguml
