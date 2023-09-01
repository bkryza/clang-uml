#include <concepts>
#include <string>

namespace clanguml {
namespace t00056 {

template <typename T, typename L>
concept greater_than_simple = sizeof(T) > sizeof(L);

template <typename T, typename P>
concept greater_than_with_requires =
    requires(T l, P r) { sizeof(l) > sizeof(r); };

// Constraint expression
template <typename T>
concept max_four_bytes = sizeof(T) <= 4;

// Simple requirement
template <typename T>
concept iterable = requires(T container) {
                       container.begin();
                       container.end();
                   };

// Type requirement
template <typename T>
concept has_value_type = requires { typename T::value_type; };

template <typename T>
concept convertible_to_string =
    max_four_bytes<T> && requires(T s) {
                             std::string{s};
                             {
                                 std::to_string(s)
                             } noexcept;
                             {
                                 std::to_string(s)
                                 } -> std::same_as<std::string>;
                         };

// Compound requirement
// ...

// Combined concept
template <typename T>
concept iterable_with_value_type = iterable<T> && has_value_type<T>;

template <typename T>
concept iterable_or_small_value_type =
    iterable_with_value_type<T> || max_four_bytes<T>;

// Simple type constraint
template <max_four_bytes T> struct A {
    T a;
};

// Requires constant expression
template <typename T>
    requires iterable_or_small_value_type<T>
struct B {
    T b;
};

// Anonymous concept requirement (TODO)
template <convertible_to_string T>
    requires requires(T t) {
                 --t;
                 t--;
             }
struct C {
    T c;
};

template <iterable T1, typename T2, iterable T3, typename T4, typename T5>
    requires max_four_bytes<T2> && max_four_bytes<T5>
struct D { };

template <typename T1, typename T2, typename T3>
    requires greater_than_with_requires<T1, T3>
struct E {
    T1 e1;
    T2 e2;
    T3 e3;
};

template <typename T1, typename T2, typename T3>
    requires greater_than_simple<T1, T3>
struct F {
    T1 f1;
    T2 f2;
    T3 f3;
};
}
}