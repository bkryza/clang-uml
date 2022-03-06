/**
 * src/util/util.h
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
#pragma once

#include <spdlog/spdlog.h>

#include <algorithm>
#include <string.h>
#include <string>
#include <type_traits>
#include <vector>

namespace clanguml {
namespace util {

std::string ltrim(const std::string &s);
std::string rtrim(const std::string &s);
std::string trim(const std::string &s);

#define __FILENAME__                                                           \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_ERROR(fmt__, ...)                                                  \
    spdlog::error(std::string("[{}:{}] ") + fmt__, __FILENAME__, __LINE__,     \
        ##__VA_ARGS__)

#define LOG_WARN(fmt__, ...)                                                   \
    spdlog::warn(std::string("[{}:{}] ") + fmt__, __FILENAME__, __LINE__,      \
        ##__VA_ARGS__)

#define LOG_INFO(fmt__, ...)                                                   \
    spdlog::info(std::string("[{}:{}] ") + fmt__, __FILENAME__, __LINE__,      \
        ##__VA_ARGS__)

#define LOG_DBG(fmt__, ...)                                                    \
    spdlog::debug(std::string("[{}:{}] ") + fmt__, __FILENAME__, __LINE__,     \
        ##__VA_ARGS__)

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

std::string join(const std::vector<std::string> &toks, std::string delimiter);

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
// std::string ns_relative(
//    const std::vector<std::string> &namespaces, const std::string &n);

/**
 * @brief Remove any qualifiers (e.g. const) from type.
 *
 * @param s String spelling of the type.
 *
 * @return Unqualified type spelling.
 */
std::string unqualify(const std::string &s);

/**
 * @brief Abbreviate string to max_length, and replace last 3 characters
 *        with ellipsis.
 *
 * @param s Input string
 * @param max_length Maximum length
 * @return Abbreviated string
 */
std::string abbreviate(const std::string &s, const unsigned int max_length);

/**
 * @brief Find element alias in Puml note
 *
 * Finds aliases of the form @A(entity_name) in the Puml notes
 * or directives.
 * The match, if any, is returned in the result tuple:
 *   (entity_name, offset, length)
 *
 * @return True if match was found
 */
bool find_element_alias(
    const std::string &input, std::tuple<std::string, size_t, size_t> &result);

/**
 * @brief Find and replace in string
 *
 * Replaces all occurences of pattern with replace_with in input string.
 *
 * @return True if at least on replacement was made
 */
bool replace_all(
    std::string &input, std::string pattern, std::string replace_with);

/**
 * @brief Appends a vector to a vector.
 *
 * @tparam T
 * @param l
 * @param r
 */
template <typename T> void append(std::vector<T> &l, const std::vector<T> &r)
{
    l.insert(l.end(), r.begin(), r.end());
}

/**
 * @brief Checks if vector starts with a prefix.
 *
 * @tparam T
 * @param col
 * @param prefix
 * @return
 */
template <typename T>
bool starts_with(const std::vector<T> &col, const std::vector<T> &prefix)
{
    if (prefix.size() > col.size())
        return false;

    return std::vector<std::string>(prefix.begin(), prefix.end()) ==
        std::vector<std::string>(col.begin(), col.begin() + prefix.size());
}

/**
 * @brief Removes prefix sequence of elements from the beggining of col.
 *
 * @tparam T
 * @param col
 * @param prefix
 */
template <typename T>
void remove_prefix(std::vector<T> &col, const std::vector<T> &prefix)
{
    if (!starts_with(col, prefix))
        return;

    col = std::vector<T>(col.begin() + prefix.size(), col.end());
}

/**
 * Returns true if element exists in container.
 *
 * @tparam T
 * @tparam E
 * @param container
 * @param element
 * @return
 */
template <typename T, typename E>
bool contains(const T &container, const E &element)
{
    if constexpr (std::is_pointer_v<E>) {
        return std::find_if(container.begin(), container.end(),
                   [&element](const auto &e) { return *e == *element; }) !=
            container.end();
    }
    else if constexpr (std::is_same_v<std::remove_cv_t<T>, std::string>) {
        return container.find(element) != std::string::npos;
    }
    else {
        return std::find(container.begin(), container.end(), element) !=
            container.end();
    }
}

} // namespace util
} // namespace clanguml