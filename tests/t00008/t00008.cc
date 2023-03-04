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

template <typename T> struct Vector {
    std::vector<T> values;
};

template <typename T, template <typename> typename C> struct B {
    C<T> template_template;
};

struct D {
    B<int, Vector> ints;

    template <typename... Items> D(std::tuple<Items...> * /*items*/) { }

    void add(int i) { ints.template_template.values.push_back(i); }
};

struct E {
    template <typename ET> struct nested_template {
        using DT = ET;

        static DT *get(ET *d) { return d; }
    };
};

template <> struct E::nested_template<char> {
    using DeclType = char;

    static DeclType *getDecl(char *c) { return c; }
};

} // namespace t00008
} // namespace clanguml
