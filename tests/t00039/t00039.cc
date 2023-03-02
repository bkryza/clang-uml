#include <string>

namespace clanguml::t00039 {
struct B { };

struct C { };

struct D { };

struct E { };

namespace ns1 {
struct BB : public B { };
} // namespace ns1

struct CD : public C, public D { };

struct DE : public D, public E { };

struct CDE : public C, public D, public E { };

struct A { };

struct AA : public A { };

struct AAA : public AA {
    B *b;
};

namespace ns2 {
struct AAAA : public virtual AAA { };
} // namespace ns2

namespace detail {
struct AA : public A { };
} // namespace detail

namespace ns3 {
template <typename T> struct F {
    T *t;
};

template <typename T, typename M> struct FF : public F<T> {
    M *m;
};

template <typename T, typename M> struct FE : public F<T> {
    M *m;
};

template <typename T, typename M, typename N> struct FFF : public FF<T, M> {
    N *n;
};

} // namespace ns3
} // namespace clanguml::t00039
