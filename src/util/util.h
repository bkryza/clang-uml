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

std::string namespace_relative(
    const std::vector<std::string> &namespaces, const std::string &n)
{
    for (const auto &ns : namespaces) {
        if(ns.empty())
            continue;

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
