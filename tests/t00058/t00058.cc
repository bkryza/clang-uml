#include <string>
#include <type_traits>
#include <vector>

// Based on a blog post:
// https://andreasfertig.blog/2020/08/cpp20-concepts-testing-constrained-functions/

namespace clanguml {
namespace t00058 {

template <typename T, typename... Args> struct first_type {
    using type = T;
};

template <typename... Args>
using first_type_t = typename first_type<Args...>::type;

// TODO: Dependency of this concept on first_type<> template does not currently
//       work due to the fact that I don't know how to extract that information
//       from clang::DependentNameType to which first_type_t<> resolves to...
template <typename T, typename... Args>
concept same_as_first_type = std::is_same_v<std::remove_cvref_t<T>,
    std::remove_cvref_t<first_type_t<Args...>>>;

template <typename T, typename... Args>
    requires same_as_first_type<T, Args...>
struct A {
    std::vector<T> a;
};

template <typename T, typename P, typename... Args>
    requires same_as_first_type<T, Args...>
struct B {
    std::vector<T> b;
    P bb;
};

struct R {
    A<int, int, double, std::string> aa;
    B<int, std::string, int, double, A<int, int>> bb;
};

}
}