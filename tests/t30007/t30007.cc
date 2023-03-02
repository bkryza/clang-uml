namespace clanguml {
namespace t30007 {

namespace B {
struct BB { };
}

/// \uml{note[top] Compare layout with t30006.}
namespace A {
namespace AA {
struct A1 {
    B::BB *b;
};
}
}

namespace C {
struct CC { };
}

/// \uml{note[bottom] Bottom A note.}
namespace A {
namespace AA {
struct A2 {
    C::CC *c;
};
}
}

}
}