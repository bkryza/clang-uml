#include <map>
#include <string>
#include <vector>

namespace clanguml {
namespace t20039 {

template <typename T> struct A {
    std::vector<std::vector<T>> a(T p) { return {}; }
};

struct R {
    A<int> a_int;
    A<std::vector<int>> a_intvec;
    A<std::vector<std::string>> a_stringvec;
    A<std::map<int, int>> a_intmap;
    A<std::map<std::string, std::string>> a_stringmap;

    void run()
    {
        a_int.a({});
        a_intvec.a({});
        a_stringvec.a({});
        a_intmap.a({});
        a_stringmap.a({});
    }
};

int tmain()
{
    R r;

    r.run();

    return 0;
}
}
}