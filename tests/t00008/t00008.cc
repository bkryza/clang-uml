#include <vector>

namespace clanguml {
namespace t00008 {
template <typename T, typename P = T> class A {
public:
    T value;
    T *pointer;
    T &reference;
    std::vector<P> values;
};
}
}
