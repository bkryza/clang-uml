namespace clanguml {
namespace t20028 {

struct A {
    int a() { return 0; }
    int b() { return 1; }
    int c() { return 2; }
    int d() { return 3; }
};

namespace detail {
struct B {
    int e() { return 4; }
};
} // namespace detail

int tmain()
{
    A a;
    detail::B b;

    int result{};

    result = b.e() ? b.e() : 12;

    result += a.a() ? a.b() + a.c() : a.d();

    return result;
}
}
}