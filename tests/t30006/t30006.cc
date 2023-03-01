namespace clanguml {
namespace t30006 {

namespace B {
struct BB { };
}

/// \uml{note[top] Top A note.}
namespace A {
struct A1 {
    B::BB *b;
};
}

namespace C {
struct CC { };
}

/// \uml{note[bottom] Bottom A note.}
namespace A {
struct A2 {
    C::CC *c;
};
}

}
}