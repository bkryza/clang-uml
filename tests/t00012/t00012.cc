#include <algorithm>
#include <numeric>
#include <variant>

namespace clanguml {
namespace t00012 {

template <typename T, typename... Ts> class A {
    T value;
    std::variant<Ts...> values;
};

template <int... Is> class B {
    std::array<int, sizeof...(Is)> ints;
};

class R {
    A<int, std::string, float> a1;
    A<int, std::string, bool> a2;

    B<3, 2, 1> b1;
    B<1, 1, 1, 1> b2;
};
}
}
