#include "include/t20060.h"

#include <string>
#include <vector>

namespace clanguml {
namespace t20060 {

bool is_empty(const std::string &t) { return t.empty(); }

void log() { }

void tmain()
{
    std::vector<std::string> vs;

    std::string out;

    util::for_each_if(
        vs, [](const auto &s) { return !is_empty(s); },
        [&](const auto &s) {
            log();
            out += s;
        });
}
}
}