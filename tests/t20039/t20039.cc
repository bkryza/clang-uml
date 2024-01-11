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
    A<std::map<int, int>> a_intmap;

    void run()
    {
        a_int.a({});
        a_intvec.a({});
        a_intmap.a({});
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