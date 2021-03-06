#include <array>
#include <vector>

namespace clanguml {
namespace t00008 {

using CMP = bool (*)(const int, const int);
template <typename T, typename P = T, CMP = nullptr, int N = 3> class A {
public:
    T value;
    T *pointer;
    T &reference;
    std::vector<P> values;
    std::array<int, N> ints;

    CMP comparator;
};
}
}
