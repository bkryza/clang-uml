namespace clanguml {
namespace t00015 {

namespace ns1 {
inline namespace ns2_v1_0_0 {
class A { };
}

namespace ns2_v0_9_0 {
class [[deprecated]] A { };
}

namespace {
class Anon final : public A { };
}
} // namespace ns1

namespace ns3 {

namespace ns1::ns2 {
class Anon : public t00015::ns1::A { };
}

class B : public ns1::ns2::Anon { };
}
} // namespace t00015
} // namespace clanguml
