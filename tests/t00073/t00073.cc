namespace clanguml {
namespace t00073 {
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

struct R {
    Overload<AHandler, BHandler> dispatch;
};
}
}