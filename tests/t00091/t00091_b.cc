#include <string>

namespace clanguml::t00091 {
struct B {
    int value;
};

template <class T> struct C {
    T value;
};

template <> struct C<std::string> {
    std::string value;
};

enum class D { one, two, three };
} // namespace clanguml::t00091