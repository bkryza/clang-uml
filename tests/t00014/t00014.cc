#include <algorithm>
#include <ios>
#include <map>
#include <numeric>
#include <string>
#include <type_traits>
#include <variant>

namespace clanguml {
namespace t00014 {

template <typename T, typename P> struct A {
    T t;
    P p;
};

template <typename T> using AString = A<T, std::string>;

using AIntString = AString<int>;
using AStringString = AString<std::string>;
using BStringString = AStringString;

class R {
    A<bool, std::string> boolstring;
    AString<float> floatstring;
    AIntString intstring;
    BStringString stringstring;
};
}
}
