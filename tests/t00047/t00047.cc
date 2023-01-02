#include <type_traits>

namespace clanguml {
namespace t00047 {

template <typename... Ts> struct conditional_t;

template <typename Else> struct conditional_t<Else> {
    using type = Else;
};

template <typename Result, typename... Tail>
struct conditional_t<std::true_type, Result, Tail...> {
    using type = Result;
};

template <typename Result, typename... Tail>
struct conditional_t<std::false_type, Result, Tail...> {
    using type = typename conditional_t<Tail...>::type;
};

template <typename... Ts>
using conditional = typename conditional_t<Ts...>::type;

} // namespace t00047
} // namespace clanguml