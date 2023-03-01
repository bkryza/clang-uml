#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace thirdparty {
namespace ns1 {
enum class color_t { red, green, blue };

struct E { };
} // namespace ns1
namespace ns2 {
struct F { };
} // namespace ns2
} // namespace thirdparty

namespace clanguml {
namespace t00038 {

enum class property_t { property_a, property_b, property_c };

struct A { };
struct B { };
struct C { };

struct key_t {
    std::string key;
};

template <typename T> struct map { };

using namespace thirdparty::ns1;

template <> struct map<std::integral_constant<color_t, color_t::red>> : E { };

template <>
struct map<std::integral_constant<clanguml::t00038::property_t,
    clanguml::t00038::property_t::property_a>> : A { };

template <>
struct map<std::vector<
    std::integral_constant<t00038::property_t, t00038::property_t::property_b>>>
    : B { };

template <>
struct map<std::map<key_t,
    std::vector<std::integral_constant<property_t, property_t::property_c>>>>
    : C { };

} // namespace t00038
} // namespace clanguml
