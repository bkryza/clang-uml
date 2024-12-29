namespace thirdparty {
namespace lib1 {
class A { };
}
namespace lib2 {
class B { };
}
}

namespace clanguml {
namespace t30017 {
namespace thirdparty {
namespace lib1 {
class A {
    ::thirdparty::lib1::A *a;
};
}
namespace lib3 {
class C {
    ::thirdparty::lib2::B *b;
};
}
namespace lib4 {
class D { };
}
}
}
}