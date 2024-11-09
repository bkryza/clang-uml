#include <algorithm>
#include <iterator>

namespace clanguml {
namespace t20060 {

namespace util {

template <typename T, typename C, typename F>
void for_each_if(const T &collection, C &&cond, F &&func)
{
    std::for_each(std::begin(collection), std::end(collection),
        [cond = std::forward<decltype(cond)>(cond),
            func = std::forward<decltype(func)>(func)](auto &e) mutable {
            if (cond(e))
                func(e);
        });
}
}

}
}