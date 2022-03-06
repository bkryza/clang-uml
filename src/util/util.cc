/**
 * src/util/util.cc
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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
#include "util.h"

#include <spdlog/spdlog.h>

#include <regex>

namespace clanguml {
namespace util {

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s) { return rtrim(ltrim(s)); }

std::vector<std::string> split(std::string str, std::string delimiter)
{
    std::vector<std::string> result;

    if (!contains(str, delimiter))
        result.push_back(str);
    else
        while (str.size()) {
            int index = str.find(delimiter);
            if (index != std::string::npos) {
                result.push_back(str.substr(0, index));
                str = str.substr(index + delimiter.size());
            }
            else {
                if (!str.empty())
                    result.push_back(str);
                str = "";
            }
        }

    return result;
}

std::string join(const std::vector<std::string> &toks, std::string delimiter)
{
    return fmt::format("{}", fmt::join(toks, delimiter));
}

/*
std::string ns_relative(
    const std::vector<std::string> &namespaces, const std::string &n)
{
    std::vector<std::string> namespaces_sorted{namespaces};

    std::sort(namespaces_sorted.rbegin(), namespaces_sorted.rend());

    auto res = n;

    for (const auto &ns : namespaces_sorted) {
        if (ns.empty())
            continue;

        if (n == ns)
            return split(n, "::").back();

        auto ns_prefix = ns + "::";
        auto it = res.find(ns_prefix);
        while (it != std::string::npos) {
            res.erase(it, ns_prefix.size());
            it = res.find(ns_prefix);
        }
    }
    return res;
}
*/

std::string unqualify(const std::string &s)
{
    auto toks = clanguml::util::split(s, " ");
    const std::vector<std::string> qualifiers = {"static", "const", "volatile",
        "register", "constexpr", "mutable", "struct", "enum"};

    toks.erase(toks.begin(),
        std::find_if(toks.begin(), toks.end(), [&qualifiers](const auto &t) {
            return std::count(qualifiers.begin(), qualifiers.end(), t) == 0;
        }));

    return fmt::format("{}", fmt::join(toks, " "));
}

std::string abbreviate(const std::string &s, const unsigned int max_length)
{
    if (s.size() <= max_length)
        return s;

    auto res = s;

    res.resize(max_length);

    if (res.size() > 3) {
        res.replace(res.size() - 3, 3, 3, '.');
    }

    return res;
}

bool find_element_alias(
    const std::string &input, std::tuple<std::string, size_t, size_t> &result)
{

    std::regex alias_regex("(@A\\([^\\).]+\\))");

    auto alias_it =
        std::sregex_iterator(input.begin(), input.end(), alias_regex);
    auto end_it = std::sregex_iterator();

    if (alias_it == end_it)
        return false;

    std::smatch match = *alias_it;
    std::string alias = match.str().substr(3, match.str().size() - 4);

    std::get<0>(result) = alias;
    std::get<1>(result) = match.position();
    std::get<2>(result) = match.length();

    return true;
}

bool replace_all(
    std::string &input, std::string pattern, std::string replace_with)
{
    bool replaced{false};

    auto pos = input.find(pattern);
    while (pos < input.size()) {
        input.replace(pos, pattern.size(), replace_with);
        pos = input.find(pattern, pos + replace_with.size());
        replaced = true;
    }

    return replaced;
}

}
}
