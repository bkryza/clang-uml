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

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <filesystem>
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
    spdlog::get("console")->error(std::string("[{}:{}] ") + fmt__,             \
        __FILENAME__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(fmt__, ...)                                                   \
    spdlog::get("console")->warn(std::string("[{}:{}] ") + fmt__,              \
        __FILENAME__, __LINE__, ##__VA_ARGS__)

#define LOG_INFO(fmt__, ...)                                                   \
    spdlog::get("console")->info(std::string("[{}:{}] ") + fmt__,              \
        __FILENAME__, __LINE__, ##__VA_ARGS__)

#define LOG_DBG(fmt__, ...)                                                    \
    spdlog::get("console")->debug(std::string("[{}:{}] ") + fmt__,             \
        __FILENAME__, __LINE__, ##__VA_ARGS__)

/**
 * @brief Setup spdlog logger.
 *
 * @param verbose Whether the logging should be verbose or not.
 */
void setup_logging(bool verbose);

std::string get_process_output(const std::string &command);

std::string get_env(const std::string &name);

bool is_git_repository();

std::string get_git_branch();

std::string get_git_revision();

std::string get_git_commit();

std::string get_git_toplevel_dir();

/**
 * @brief Split a string using delimiter
 *
 * Basic string split function, because C++ stdlib does not have one.
 * In case the string does not contain the delimiter, the original
 * string is returned as the only element of the vector.
 *
 * @param str String to split
 * @param delimiter Delimiter string
 * @param skip_empty Skip empty toks between delimiters if true
 *
 * @return Vector of string tokens.
 */
std::vector<std::string> split(
    std::string str, std::string_view delimiter, bool skip_empty = true);

std::string join(
    const std::vector<std::string> &toks, std::string_view delimiter);

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
 * @brief Checks if collection starts with a prefix.
 *
 * @tparam T e.g. std::vector<std::string>
 * @param col Collection to be checked against prefix
 * @param prefix Container, which specifies the prefix
 * @return true if first prefix.size() elements of col are equal to prefix
 */
template <typename T> bool starts_with(const T &col, const T &prefix)
{
    if (prefix.size() > col.size())
        return false;

    return std::search(col.begin(), col.end(), prefix.begin(), prefix.end()) ==
        col.begin();
}

template <>
bool starts_with(
    const std::filesystem::path &path, const std::filesystem::path &prefix);

template <> bool starts_with(const std::string &s, const std::string &prefix);

template <typename T> bool ends_with(const T &value, const T &suffix);

template <> bool ends_with(const std::string &value, const std::string &suffix);

template <typename T>
bool ends_with(const std::vector<T> &col, const std::vector<T> &suffix)
{
    if (suffix.size() > col.size())
        return false;

    return std::vector<std::string>(suffix.rbegin(), suffix.rend()) ==
        std::vector<std::string>(col.rbegin(), col.rbegin() + suffix.size());
}

/**
 * @brief Removes prefix sequence of elements from the beginning of col.
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

template <typename T, typename F> void for_each(const T &collection, F &&func)
{
    std::for_each(std::begin(collection), std::end(collection),
        std::forward<decltype(func)>(func));
}

template <typename T, typename C, typename F>
void for_each_if(const T &collection, C &&cond, F &&func)
{
    std::for_each(std::begin(collection), std::end(collection),
        [cond = std::forward<decltype(cond)>(cond),
            func = std::forward<decltype(func)>(func)](const auto &e) {
            if (cond(e))
                func(e);
        });
}

} // namespace util
} // namespace clanguml