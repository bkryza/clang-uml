#pragma once

#include <string>
#include <vector>

namespace clanguml {
namespace util {

std::string namespace_relative(
    const std::vector<std::string> &namespaces, const std::string &n)
{
    for (const auto &ns : namespaces) {
        if (n == ns)
            return "";

        if (n.find(ns) == 0) {
            if (n.size() <= ns.size() + 2)
                return "";

            return n.substr(ns.size() + 2);
        }
    }

    return n;
}
}
}
