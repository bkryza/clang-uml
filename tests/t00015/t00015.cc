namespace clanguml {
namespace t00015 {

namespace ns1::ns2 {
class A {
};

namespace {
class Anon final : public A {
};
}
}

namespace ns3 {

namespace ns1::ns2 {
class Anon : public t00015::ns1::ns2::A {
};
}

class B : public ns1::ns2::Anon {
};
}
}
}
