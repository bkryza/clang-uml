#include <map>
#include <string>
#include <vector>

namespace clanguml {
namespace t00081_detail {
struct C { };
} // namespace t00081_detail
namespace t00081 {
struct A {
    std::vector<std::string> as;
    std::string s;
    std::map<std::string, std::string> ms;

    t00081_detail::C *c;
};
} // namespace t00081
} // namespace clanguml