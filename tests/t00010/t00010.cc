#include <string>
#include <vector>

namespace clanguml {
namespace t00010 {

template <typename T, typename P> class A {
public:
    T first;
    P second;
};

template <typename T> class B {
public:
    A<T, std::string> astring;
};

class C {
public:
    B<int> aintstring;
};
} // namespace t00010
} // namespace clanguml
