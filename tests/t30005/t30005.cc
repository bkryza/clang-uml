namespace clanguml {
namespace t30005 {

namespace A::AA::AAA {
struct C1 { };
}

namespace B::BB::BBB {
namespace A6 = A::AA::AAA;
namespace ASix = A6;
struct C2 {
    ASix::C1 *cb;
};
}

namespace C::CC::CCC {
namespace A6 = A::AA::AAA;
namespace ASix = A6;
using ADSix = ASix::C1;
struct C2 {
    ADSix *cc;
};
}
}

}
