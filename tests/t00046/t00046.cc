#include <cstdint>
#include <vector>

class A { };

class AA { };

namespace ns1 {

class A { };

namespace ns2 {

class A { };

class B : public A { };

class C : public ns1::A { };

class D : public ns1::ns2::A { };

class E : public ::A { };

class R {
public:
    A *a;
    ns1::A *ns1_a;
    ns1::ns2::A *ns1_ns2_a;
    ::A *root_a;
    std::vector<std::uint8_t> i;

    void foo(::AA &aa) { (void)aa; }
};
} // namespace ns2
} // namespace ns1
