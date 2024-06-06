/**
 * @file src/util/util.cc
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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
#if __has_include(<sys/utsname.h>)
#include <sys/utsname.h>
#endif

namespace clanguml::util {

static const auto WHITESPACE = " \n\r\t\f\v";

namespace {
class pipe_t {
public:
    explicit pipe_t(const std::string &command, int &result)
        : result_{result}
        ,
#if defined(__linux) || defined(__unix) || defined(__APPLE__)
        pipe_{popen(fmt::format("{} 2>&1", command).c_str(), "r")}
#elif defined(_WIN32)
        pipe_{_popen(command.c_str(), "r")}
#endif
    {
    }

    ~pipe_t() { reset(); }

    operator bool() const { return pipe_ != nullptr; }

    FILE *get() const { return pipe_; }

    void reset()
    {
        if (pipe_ == nullptr)
            return;

#if defined(__linux) || defined(__unix) || defined(__APPLE__)
        result_ = pclose(pipe_);
#elif defined(_WIN32)
        result_ = _pclose(pipe_);
#endif
        pipe_ = nullptr;
    }

private:
    int &result_;
    FILE *pipe_;
};
} // namespace

std::string get_process_output(const std::string &command)
{
    constexpr size_t kBufferSize{1024};
    std::array<char, kBufferSize> buffer{};
    std::string output;
    int result{EXIT_FAILURE};

    pipe_t pipe{command, result};

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        output += buffer.data();
    }

    pipe.reset();

    if (result != EXIT_SUCCESS) {
        throw std::runtime_error(
            fmt::format("External command '{}' failed: {}", command, output));
    }

    return output;
}

void check_process_output(const std::string &command)
{
    constexpr size_t kBufferSize{1024};
    std::array<char, kBufferSize> buffer{};
    int result{EXIT_FAILURE};
    std::string output;
    pipe_t pipe{command, result};

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        output += buffer.data();
    }

    pipe.reset();

    if (result != EXIT_SUCCESS) {
        throw std::runtime_error(
            fmt::format("External command '{}' failed: {}", command, output));
    }
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

    std::string output;

    try {
        output = get_process_output("git rev-parse --git-dir");
    }
    catch (std::runtime_error &e) {
        return false;
    }

    return contains(trim(output), ".git");
}

std::string run_git_command(
    const std::string &cmd, const std::string &env_override)
{
    auto env = get_env(env_override);

    if (!env.empty())
        return env;

    std::string output;

    try {
        output = get_process_output(cmd);
    }
    catch (std::runtime_error &e) {
        return {};
    }

    return trim(output);
}

std::string get_git_branch()
{
    return run_git_command(
        "git rev-parse --abbrev-ref HEAD", "CLANGUML_GIT_BRANCH");
}

std::string get_git_revision()
{
    return run_git_command(
        "git describe --tags --always", "CLANGUML_GIT_REVISION");
}

std::string get_git_commit()
{
    return run_git_command("git rev-parse HEAD", "CLANGUML_GIT_COMMIT");
}

std::string get_git_toplevel_dir()
{
    return run_git_command(
        "git rev-parse --show-toplevel", "CLANGUML_GIT_TOPLEVEL_DIR");
}

std::string get_os_name()
{
#ifdef _WIN32
    return "Windows, 32-bit";
#elif _WIN64
    return "Windows, 64-bit";
#elif __has_include(<sys/utsname.h>)
    struct utsname utsn; // NOLINT
    uname(&utsn);
    return fmt::format("{} {} {}", utsn.sysname, utsn.machine, utsn.release);
#elif __linux__
    return "Linux";
#elif __APPLE__ || __MACH__
    return "macOS";
#elif __FreeBSD__
    return "FreeBSD";
#elif __unix__ || __unix
    return "Unix";
#else
    return "Unknown";
#endif
}

std::string ltrim(const std::string &s)
{
    const size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
    const size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim_typename(const std::string &s)
{
    auto res = trim(s);
    if (res.find("typename ") == 0)
        return res.substr(strlen("typename "));

    return res;
}

std::string trim(const std::string &s) { return rtrim(ltrim(s)); }

std::vector<std::string> split(
    std::string str, std::string_view delimiter, bool skip_empty)
{
    std::vector<std::string> result;

    if (!contains(str, delimiter)) {
        if (!str.empty())
            result.push_back(std::move(str));
        else if (!skip_empty)
            result.push_back(std::move(str));
    }
    else
        while (static_cast<unsigned int>(!str.empty()) != 0U) {
            auto index = str.find(delimiter);
            if (index != std::string::npos) {
                auto tok = str.substr(0, index);
                if (!tok.empty())
                    result.push_back(std::move(tok));
                else if (!skip_empty)
                    result.push_back(std::move(tok));

                str = str.substr(index + delimiter.size());
            }
            else {
                if (!str.empty())
                    result.push_back(std::move(str));
                else if (!skip_empty)
                    result.push_back(std::move(str));
                str = "";
            }
        }

    return result;
}

std::vector<std::string> split_isspace(std::string str)
{
    std::vector<std::string> result;

    while (static_cast<unsigned int>(!str.empty()) != 0U) {
        auto index = std::find_if(
            str.begin(), str.end(), [](auto c) { return std::isspace(c); });
        if (index != str.end()) {
            auto tok = str.substr(0, std::distance(str.begin(), index));
            if (!tok.empty())
                result.push_back(std::move(tok));
            str = str.substr(std::distance(str.begin(), index) + 1);
        }
        else {
            if (!str.empty())
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
    const std::regex alias_regex(R"((@A\([^\).]+\)))");

    auto alias_it =
        std::sregex_iterator(input.begin(), input.end(), alias_regex);
    auto end_it = std::sregex_iterator();

    if (alias_it == end_it)
        return false;

    const std::smatch &match = *alias_it;
    const std::string alias = match.str().substr(3, match.str().size() - 4);

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

std::filesystem::path ensure_path_is_absolute(
    const std::filesystem::path &p, const std::filesystem::path &root)
{
    if (p.is_absolute())
        return p;

    auto result = root / p;
    result = result.lexically_normal();
    result.make_preferred();

    return result;
}

bool is_relative_to(
    const std::filesystem::path &child, const std::filesystem::path &parent)
{
    if (child.has_root_directory() != parent.has_root_directory())
        return false;

    return starts_with(weakly_canonical(child), weakly_canonical(parent));
}

std::string format_message_comment(const std::string &comment, unsigned width)
{
    if (width == 0)
        return comment;

    std::string result;

    if (comment.empty())
        return result;

    auto tokens = split_isspace(comment);

    if (tokens.empty())
        return result;

    unsigned current_line_length{0};
    for (const auto &token : tokens) {
        if (current_line_length < width) {
            result += token;
            result += ' ';
        }
        else {
            result.back() = '\n';
            current_line_length = 0;
            result += token;
            result += ' ';
        }

        current_line_length += token.size() + 1;
    }

    result.pop_back();

    return result;
}

} // namespace clanguml::util
