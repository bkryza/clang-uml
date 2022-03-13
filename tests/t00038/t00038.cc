#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace clanguml {
namespace t00038 {

enum class property_t { property_a, property_b, property_c };

struct A {
};
struct B {
};
struct C {
};

struct key_t {
    std::string key;
};

template <typename T> struct map;

template <>
struct map<std::integral_constant<clanguml::t00038::property_t,
    clanguml::t00038::property_t::property_a>> : A {
};

template <>
struct map<std::vector<
    std::integral_constant<t00038::property_t, t00038::property_t::property_b>>>
    : B {
};

template <>
struct map<std::map<key_t,
    std::vector<std::integral_constant<property_t, property_t::property_c>>>>
    : C {
};

} // namespace t00038
} // namespace clanguml
