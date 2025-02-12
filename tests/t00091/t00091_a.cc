#include <string>

namespace clanguml::t00091 {
struct B;

template <class T> struct C;

template <> struct C<std::string>;

enum class D;

struct R {
    B *b;
    C<int> *c;
    D d;
};
} // namespace clanguml::t00091