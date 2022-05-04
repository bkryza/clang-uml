#include <algorithm>
#include <functional>
#include <ios>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

/**
 * These should not be include as they are not
 * in ns clanguml::t00014
 */
template <typename T> struct clanguml_t00014_A {
    T value;
};

using clanguml_t00014_AString = clanguml_t00014_A<std::string>;

namespace clanguml {
namespace t00014 {

template <typename T, typename P> struct A {
    T t;
    P p;
};

template <typename T> using AString = A<T, std::string>;
template <typename T> using AStringPtr = A<T, std::unique_ptr<std::string>>;

template <class T> using VectorPtr = std::unique_ptr<std::vector<T>>;
template <class T> using APtr = std::unique_ptr<A<double, T>>;
template <class T> using ASharedPtr = std::shared_ptr<A<double,T>>;
 template <class T, class U> using AAPtr =
             std::unique_ptr<std::pair<A<double,T>, A<long,U>>>;

template <typename... T> using GeneralCallback = std::function<void(T..., int)>;
using VoidCallback = GeneralCallback<>;

struct B {
    std::string value;
};

using BVector = std::vector<B>;
using BVector2 = BVector;

using AIntString = AString<int>;
using AStringString = AString<std::string>;
using BStringString = AStringString;

template <typename T> using PairPairBA = std::pair<std::pair<B, A<long,T>>, long>;

class R {
    PairPairBA<bool> bapair;

    APtr<bool> abool;
    AAPtr<bool,float> aboolfloat;
    ASharedPtr<float> afloat;
    A<bool, std::string> boolstring;
    AStringPtr<float> floatstring;
    AIntString intstring;
    AStringString stringstring;
    BStringString bstringstring;

protected:
    BVector bs;

public:
    BVector2 bs2;
    GeneralCallback<AIntString> cb;
    VoidCallback vcb;
    VectorPtr<B> vps;
};
}
}
