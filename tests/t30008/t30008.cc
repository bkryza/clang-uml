namespace clanguml {
namespace t30008 {

namespace dependants {
namespace X {
}
namespace A {
struct AA { };
}
namespace B {
struct BB {
    A::AA *aa;
};
}
namespace C {
struct CC {
    B::BB *bb;
};
}

} // namespace dependants

namespace dependencies {
namespace Y {
}

namespace D {
struct DD { };
}
namespace E {
struct EE {
    D::DD *dd;
};
}
namespace F {
struct FF {
    E::EE *ee;
};
}
} // namespace dependencies

} // namespace t30008
} // namespace clanguml