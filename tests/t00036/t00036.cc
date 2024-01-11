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

} // namespace ns111
} // namespace ns11
} // namespace ns1

namespace ns2 {
namespace ns22 {

// TODO: Fix for incomplete struct C declaration "struct C;"
struct C { };

struct D { };

} // namespace ns22
} // namespace ns2

namespace ns3 {
namespace ns33 {
namespace detail {
struct DImpl : public ns2::ns22::D { };
}
} // namespace ns33
} // namespace ns3

} // namespace t00036
} // namespace clanguml
