namespace clanguml {
namespace t30003 {

namespace ns1 {
namespace ns2_v1_0_0 {
class A { };
} // namespace ns2_v1_0_0

namespace [[deprecated]] ns2_v0_9_0 {
class A { };
} // namespace ns2_v0_9_0

namespace {
class Anon final { };
} // namespace
} // namespace ns1

namespace [[deprecated]] ns3 {

namespace ns1::ns2 {
class Anon : public t30003::ns1::ns2_v1_0_0::A { };
} // namespace ns1::ns2

class B : public ns1::ns2::Anon { };
} // namespace ns3
} // namespace t30003
} // namespace clanguml