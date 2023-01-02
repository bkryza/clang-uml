#include <algorithm>
#include <array>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace clanguml {
namespace t00012 {

template <typename T, typename... Ts> class A {
    T value;
    std::variant<Ts...> values;
};

template <int... Is> class B {
    std::array<int, sizeof...(Is)> ints;
};

template <typename T, int... Is> class C {
    std::array<T, sizeof...(Is)> ints;
};

class R {
    A<int, std::string, float> a1;
    A<int, std::string, bool> a2;

    B<3, 2, 1> b1;
    B<1, 1, 1, 1> b2;

    C<std::map<int, std::vector<std::vector<std::vector<std::string>>>>, 3, 3,
        3>
        c1;
};
} // namespace t00012
} // namespace clanguml
