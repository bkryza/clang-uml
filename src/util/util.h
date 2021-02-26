/**
 * src/util/util.h
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <string>
#include <vector>

namespace clanguml {
namespace util {

std::vector<std::string> split(std::string str, std::string delimiter)
{
    std::vector<std::string> result;

    while (str.size()) {
        int index = str.find(delimiter);
        if (index != std::string::npos) {
            result.push_back(str.substr(0, index));
            str = str.substr(index + delimiter.size());
            if (str.size() == 0)
                result.push_back(str);
        }
        else {
            result.push_back(str);
            str = "";
        }
    }

    if (result.empty())
        result.push_back(str);

    return result;
}

std::string ns_relative(
    const std::vector<std::string> &namespaces, const std::string &n)
{
    std::vector<std::string> namespaces_sorted{namespaces};

    std::sort(namespaces_sorted.rbegin(), namespaces_sorted.rend());

    for (const auto &ns : namespaces_sorted) {
        if (ns.empty())
            continue;

        if (n == ns)
            return split(n, "::").back();

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
