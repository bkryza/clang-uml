#include <cstddef>
#include <tuple>

namespace clanguml {
namespace t00064 {

// Loosely based on
template <typename... Ts> struct type_list { };

template <typename Ret, typename Arg, typename... Ts>
struct type_list<Ret (*)(Arg &&), Ts...> { };

template <typename T, typename... Ts> struct type_list<const T, Ts...> { };

template <typename> struct head;
template <typename Head, typename... Tail>
struct head<type_list<Head, Tail...>> {
    using type = Head;
};

template <typename... Type> using first_t = type_list<Type...>;

template <typename... Type> using second_t = type_list<Type...>;

template <typename, typename> class type_group_pair;
template <typename... First, typename... Second>
class type_group_pair<first_t<First...>, second_t<Second...>> {
    static constexpr size_t size = sizeof...(First) + sizeof...(Second);
};

template <typename T> struct optional_ref { };

template <typename, typename, typename> class type_group_pair_it;
template <typename It, typename... First, typename... Second>
class type_group_pair_it<It, first_t<First...>, second_t<Second...>> {
public:
    using value_type =
        decltype(std::tuple_cat(std::make_tuple(*std::declval<It>()),
            std::declval<First>().get_as_tuple({})...,
            std::declval<Second>().get_as_tuple({})...));

    using ref_t = optional_ref<value_type>;

    ref_t get(unsigned i) { return {}; }

    const value_type *getp(unsigned i) { return nullptr; }

    constexpr unsigned find(value_type const &v) { return 0; }
};

struct A { };
struct B { };
struct C { };

class R {
public:
    type_list<A, bool, int> aboolint;
    type_group_pair<type_list<float, double>, type_list<A, B, C>> abc;
};
}
}