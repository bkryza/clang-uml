/**
 * src/util/util.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

namespace clanguml::util {

static const auto WHITESPACE = " \n\r\t\f\v";

void setup_logging(int verbose)
{
    auto console =
        spdlog::stdout_color_mt("console", spdlog::color_mode::automatic);

    console->set_pattern("[%^%l%^] [tid %t] %v");

    if (verbose == 0) {
        console->set_level(spdlog::level::err);
    }
    else if (verbose == 1) {
        console->set_level(spdlog::level::info);
    }
    else if (verbose == 2) {
        console->set_level(spdlog::level::debug);
    }
    else {
        console->set_level(spdlog::level::trace);
    }
}

std::string get_process_output(const std::string &command)
{
    constexpr size_t kBufferSize{1024};
    std::array<char, kBufferSize> buffer{};
    std::string result;

#if defined(__linux) || defined(__unix) || defined(__APPLE__)
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(command.c_str(), "r"), pclose);
#elif defined(_WIN32)
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(
        _popen(command.c_str(), "r"), _pclose);
#endif

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

std::string get_env(const std::string &name)
{
#if defined(__linux) || defined(__unix)
    const char *value = std::getenv(name.c_str()); // NOLINT

    if (value == nullptr)
        return {};

    return std::string{value};
#elif defined(WINDOWS) || defined(_WIN32) || defined(WIN32)
    static constexpr auto kMaxEnvLength = 2096U;
    static char value[kMaxEnvLength];
    const DWORD ret =
        GetEnvironmentVariableA(name.c_str(), value, kMaxEnvLength);
    if (ret == 0 || ret > kMaxEnvLength)
        return {};
    else
        return value;
#else
    return {};
#endif
}

bool is_git_repository()
{
    const auto env = get_env("CLANGUML_GIT_COMMIT");

    if (!env.empty())
        return true;

    return contains(
        trim(get_process_output("git rev-parse --git-dir")), ".git");
}

std::string get_git_branch()
{
    auto env = get_env("CLANGUML_GIT_BRANCH");

    if (!env.empty())
        return env;

    return trim(get_process_output("git rev-parse --abbrev-ref HEAD"));
}

std::string get_git_revision()
{
    auto env = get_env("CLANGUML_GIT_REVISION");

    if (!env.empty())
        return env;

    return trim(get_process_output("git describe --tags --always"));
}

std::string get_git_commit()
{
    auto env = get_env("CLANGUML_GIT_COMMIT");

    if (!env.empty())
        return env;

    return trim(get_process_output("git rev-parse HEAD"));
}

std::string get_git_toplevel_dir()
{
    auto env = get_env("CLANGUML_GIT_TOPLEVEL_DIR");

    if (!env.empty())
        return env;

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

std::vector<std::string> split(
    std::string str, std::string_view delimiter, bool skip_empty)
{
    std::vector<std::string> result;

    if (!contains(str, delimiter))
        result.push_back(str);
    else
        while (static_cast<unsigned int>(!str.empty()) != 0U) {
            auto index = str.find(delimiter);
            if (index != std::string::npos) {
                auto tok = str.substr(0, index);
                if (!tok.empty() || !skip_empty)
                    result.push_back(std::move(tok));
                str = str.substr(index + delimiter.size());
            }
            else {
                if (!str.empty() || !skip_empty)
                    result.push_back(str);
                str = "";
            }
        }

    return result;
}

std::string join(
    const std::vector<std::string> &toks, std::string_view delimiter)
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
    std::regex alias_regex(R"((@A\([^\).]+\)))");

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

bool replace_all(std::string &input, const std::string &pattern,
    const std::string &replace_with)
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

template <>
bool starts_with(
    const std::filesystem::path &path, const std::filesystem::path &prefix)
{
    auto normal_path = std::filesystem::path();
    auto normal_prefix = std::filesystem::path();

    for (const auto &element : path.lexically_normal()) {
        if (!element.empty())
            normal_path /= element;
    }

    for (const auto &element : prefix.lexically_normal()) {
        if (!element.empty())
            normal_prefix /= element;
    }

    auto normal_path_str = normal_path.string();
    auto normal_prefix_str = normal_prefix.string();
    return std::search(normal_path_str.begin(), normal_path_str.end(),
               normal_prefix_str.begin(),
               normal_prefix_str.end()) == normal_path_str.begin();
}

template <> bool starts_with(const std::string &s, const std::string &prefix)
{
    return s.rfind(prefix, 0) == 0;
}

template <> bool ends_with(const std::string &value, const std::string &suffix)
{
    if (suffix.size() > value.size())
        return false;
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

std::size_t hash_seed(std::size_t seed)
{
    constexpr auto kSeedStart{0x6a3712b5};
    constexpr auto kSeedShiftFirst{6};
    constexpr auto kSeedShiftSecond{2};

    return kSeedStart + (seed << kSeedShiftFirst) + (seed >> kSeedShiftSecond);
}

std::string path_to_url(const std::filesystem::path &p)
{
    std::vector<std::string> path_tokens;
    auto it = p.begin();
    if (p.has_root_directory()) {
#ifdef _MSC_VER
        // On Windows convert the root path using its drive letter, e.g.:
        //   C:\A\B\include.h -> /c/A/B/include.h
        if (p.root_name().string().size() > 1) {
            if (p.is_absolute()) {
                path_tokens.push_back(std::string{
                    std::tolower(p.root_name().string().at(0), std::locale())});
            }
            it++;
        }
#endif
        it++;
    }

    for (; it != p.end(); it++)
        path_tokens.push_back(it->string());

    if (p.has_root_directory())
        return fmt::format("/{}", fmt::join(path_tokens, "/"));

    return fmt::format("{}", fmt::join(path_tokens, "/"));
}

} // namespace clanguml::util
