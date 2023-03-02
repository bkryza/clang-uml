namespace clanguml {
namespace t30003 {

namespace ns1 {
namespace ns2_v1_0_0 {
class A { };
}

namespace [[deprecated]] ns2_v0_9_0 {
class A { };
}

namespace {
class Anon final { };
}
}

namespace [[deprecated]] ns3 {

namespace ns1::ns2 {
class Anon : public t30003::ns1::ns2_v1_0_0::A { };
}

class B : public ns1::ns2::Anon { };
}
}
}