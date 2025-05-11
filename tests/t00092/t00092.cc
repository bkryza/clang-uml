#include <cstddef>
#include <map>

namespace clanguml::t00092 {

template <typename T> struct OwningPtr {
    T *ptr;
};

template <typename T> struct WeakPtr {
    T *ptr;
};

struct String {
    char *data;
    size_t length;
};

template <typename K, typename V> struct Map {
    std::map<K, V> map;
};

class A { };

class B { };

class C { };

class R {
public:
    OwningPtr<A> a;
    WeakPtr<B> b;
    Map<String, C> m;
};
} // namespace clanguml::t00092
