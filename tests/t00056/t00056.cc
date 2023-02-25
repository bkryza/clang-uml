#include <string>

namespace clanguml {
namespace t00056 {

template <typename T, typename L>
concept greater_than = sizeof(T) > sizeof(L);

template <typename T, typename P>
concept greater_than_requires = requires(T l, P r)
{
    sizeof(l) > sizeof(r);
};

// Constraint expression
template <typename T>
concept max_four_bytes = sizeof(T) <= 4;

// Simple requirement
template <typename T>
concept iterable = requires(T container)
{
    container.begin();
    container.end();
};

// Type requirement
template <typename T>
concept has_value_type = requires
{
    typename T::value_type;
};

template <typename T>
concept convertible_to_string = requires(T s)
{
    std::string{s};
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

// Anonymous concept requirement
template <typename T>
requires requires(T t)
{
    --t;
    t--;
}
struct C {
    T c;
};

}
}