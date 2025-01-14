namespace clanguml::t30018 {
namespace context {
namespace X {
}
namespace A {
struct AA { };
} // namespace A

namespace B {
struct BB {
    A::AA *aa;
};
} // namespace B

namespace C {
struct CC {
    B::BB *bb;
};
namespace Y {
namespace YY {
namespace YYY {
} // namespace YYY
} // namespace YY
} // namespace Y
} // namespace C

namespace D {
struct DD { };
} // namespace D

namespace E {
struct EE {
    B::BB *bb;
    D::DD *dd;
};
} // namespace E

namespace F {
struct FF {
    E::EE *ee;
};
} // namespace F
} // namespace context
} // namespace clanguml::t30018