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

/*
 * TODO: Handle template template properly.
 *
template <typename T, template <typename> typename C> struct B {
    C<T> template_template;
};

struct D {
    // libclang claims that the type spelling of 'ints' is 'int'...
    B<int, std::vector> ints;

    void add(int i) { ints.template_template.push_back(i); }
};
*/
}
}
