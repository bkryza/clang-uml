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

#include <algorithm>
#include <string>
#include <vector>

namespace clanguml {
namespace util {

/**
 * @brief Split a string using delimiter
 *
 * Basic string split function, because C++ stdlib does not have one.
 * In case the string does not contain the delimiter, the original
 * string is returned as the only element of the vector.
 *
 * @param str String to split
 * @param delimiter Delimiter string
 *
 * @return Vector of string tokens.
 */
std::vector<std::string> split(std::string str, std::string delimiter);

/**
 * @brief Get name of the identifier relative to a set of namespaces
 *
 * This function tries to match a given C++ identifier (e.g.
 * clanguml::util::split) to the longest namespace from the provided list
 * matching the identifier from the left.
 * If a match is found, the relative identifier is returned. If none of
 * the namespaces match the identifier or if nothing is left after
 * removing the matching namespace from the identifier, original identifier is
 * returned.
 *
 * @param namespaces List of C++ namespaces to consider
 * @param n Identifier to relativize
 *
 * @return Identifier relative to one of the matching namespaces.
 */
std::string ns_relative(
    const std::vector<std::string> &namespaces, const std::string &n);
}
}
