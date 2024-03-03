namespace clanguml {
namespace t00075 {
namespace ns1 {
namespace ns2 {

template <typename T>
concept C = requires(T t) {
                T{};
                t.e();
            };

enum class E { k1, k2 };

struct A {
    E e() const { return E::k1; };
};

class B {
public:
    E e() const { return E::k2; };
};

template <C T> class ABE {
    T a_or_b;
};

struct R {
    ABE<A> a;
    ABE<B> b;
};
} // namespace ns2
} // namespace ns1
}
}