#include <type_traits>

namespace clanguml {
//
// Based on https://github.com/facebook/folly/blob/master/folly/Unit.h
//
namespace t00034 {

struct Void {
    constexpr bool operator==(const Void & /* unused */) const { return true; }
    constexpr bool operator!=(const Void & /* unused */) const { return false; }
};

constexpr Void void_t{};

template <typename T> struct lift_void {
    using type = T;
};

template <> struct lift_void<void> {
    using type = Void;
};

//
// TODO: This is a shortcoming of libclang which parses the type of lift_void_t
// alias as unexposed, i.e. no actual reference to T can be inferred without
// manually parsing the string 'typename lift_void<T>::type'
// For now, this test validates that the visitor does not crash, the reference
// between R and A has to be provided in the configuration file
//
template <typename T> using lift_void_t = typename lift_void<T>::type;

template <typename T> struct drop_void {
    using type = T;
};

template <> struct drop_void<Void> {
    using type = void;
};

template <typename T> using drop_void_t = typename drop_void<T>::type;

struct A { };

struct R {
    lift_void_t<A> *la;
    lift_void_t<void> *lv;
};

} // namespace t00034
} // namespace clanguml
