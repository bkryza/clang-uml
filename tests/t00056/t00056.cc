#include <string>

namespace clanguml {
namespace t00056 {
// Constraint expression
template <typename T>
concept MaxFourBytes = sizeof(T) <= 4;

// Simple requirement
template <typename T>
concept Iterable = requires(T container) {
                       container.begin();
                       container.end();
                   };

// Type requirement
template <typename T>
concept HasValueType = requires { typename T::value_type; };

template <typename T>
concept ConvertibleToString = requires(T s) { std::string{s}; };

// Compound requirement
// ...

// Combined concept
template <typename T>
concept IterableWithValueType = Iterable<T> && HasValueType<T>;

template <typename T>
concept IterableWithSmallValueType =
    IterableWithValueType<T> && MaxFourBytes<T>;

// Simple type constraint
template <MaxFourBytes T> struct A {
    T a;
};

// Requires constant expression
template <typename T>
    requires MaxFourBytes<T>
struct B {
    T b;
};

// Anonymous concept requirement
template <typename T>
    requires requires(T t) {
                 --t;
                 t--;
             }
struct C {
    T c;
};

}
}