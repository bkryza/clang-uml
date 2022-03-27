namespace clanguml::t00039 {
struct B {
};

namespace ns1 {
struct BB : public B {
};
} // namespace ns1

struct A {
};

struct AA : public A {
};

struct AAA : public AA {
    B *b;
};

namespace ns2 {
struct AAAA : public AAA {
};
} // namespace ns2
} // namespace clanguml::t00039
