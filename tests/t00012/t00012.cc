#include <algorithm>
#include <numeric>
#include <variant>

namespace clanguml {
namespace t00012 {

template <typename T, typename... Ts> class A {
    T value;
    std::variant<Ts...> values;
};
}
}
