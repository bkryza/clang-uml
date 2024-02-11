namespace clanguml {
namespace t20042 {
struct A { };
struct AHandler {
    void handle(A &a) const { }
    void operator()(A &a) const { handle(a); }
};

struct B { };
struct BHandler {
    void handle(B &b) const { }
    void operator()(B &b) const { handle(b); }
};

template <typename... Bases> struct Overload : public Bases... {
    using Bases::operator()...;
};
template <typename... Bases> Overload(Bases...) -> Overload<Bases...>;

void tmain()
{
    Overload<AHandler, BHandler> dispatch;
    A a;
    B b;

    dispatch(a);
    dispatch(b);
}
}
}