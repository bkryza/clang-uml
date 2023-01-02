#include <map>
#include <string>
#include <vector>

namespace clanguml {
namespace t00049 {
template <typename T> struct A {
    T a;

    T &get_a() { return a; }
};

struct R {
    A<std::basic_string<char>> a_string;
    A<std::vector<std::string>> a_vector_string;
    A<std::map<int, int>> a_int_map;

    A<std::map<int, int>> get_int_map() { return a_int_map; }

    void set_int_map(A<std::map<int, int>> &&int_map) { a_int_map = int_map; }
};
} // namespace t00049
} // namespace clanguml