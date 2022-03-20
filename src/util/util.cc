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

void setup_logging(bool verbose)
{
    auto console =
        spdlog::stdout_color_mt("console", spdlog::color_mode::automatic);

    console->set_pattern("[%^%l%^] [tid %t] %v");

    if (verbose) {
        console->set_level(spdlog::level::debug);
    }
}

std::string get_process_output(const std::string &command)
{
    std::array<char, 1024> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

bool is_git_repository()
{
#if defined(_WIN32) || defined(_WIN64)
    return false;
#else
    return contains(
        trim(get_process_output("git rev-parse --git-dir 2> /dev/null")),
        ".git");
#endif
}

std::string get_git_branch()
{
    return trim(get_process_output("git rev-parse --abbrev-ref HEAD"));
}

std::string get_git_revision()
{
    return trim(get_process_output("git describe --tags --always"));
}

std::string get_git_commit()
{
    return trim(get_process_output("git rev-parse HEAD"));
}

std::string get_git_toplevel_dir()
{
    return trim(get_process_output("git rev-parse --show-toplevel"));
}

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
