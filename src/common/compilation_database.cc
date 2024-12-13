/**
 * @file src/common/compilation_database.cc
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

#include "compilation_database.h"
#include "util/error.h"
#include "util/query_driver_output_extractor.h"

namespace clanguml::common {

std::unique_ptr<compilation_database>
compilation_database::auto_detect_from_directory(
    const clanguml::config::config &cfg)
{
    std::string error_message;
    auto res = clang::tooling::CompilationDatabase::autoDetectFromDirectory(
        cfg.compilation_database_dir(), error_message);

    if (!error_message.empty())
        throw error::compilation_database_error(error_message);

    if (res.get() == nullptr)
        throw error::compilation_database_error(fmt::format(
            "Autodetection of compilation database from directory '{}' failed",
            cfg.compilation_database_dir()));

    // This is a workaround to determine whether Clang loaded a fixed
    // compilation database or a proper one. This cannot be done with
    // dynamic_cast if RTTI was not enabled in the LLVM build
    bool is_fixed{res->getAllFiles().empty()};

    return std::make_unique<compilation_database>(
        std::move(res), cfg, is_fixed);
}

compilation_database::compilation_database(
    std::unique_ptr<clang::tooling::CompilationDatabase> base,
    const clanguml::config::config &cfg, bool is_fixed)
    : base_{std::move(base)}
    , config_{cfg}
    , is_fixed_{is_fixed}
{
}

bool compilation_database::is_fixed() const { return is_fixed_; }

const clanguml::config::config &compilation_database::config() const
{
    return config_;
}

const clang::tooling::CompilationDatabase &compilation_database::base() const
{
    return *base_;
}

std::vector<std::string> compilation_database::getAllFiles() const
{
    return base().getAllFiles();
}

std::vector<clang::tooling::CompileCommand>
compilation_database::getCompileCommands(clang::StringRef FilePath) const
{
    auto commands = base().getCompileCommands(FilePath);

    adjust_compilation_database(commands);

    return commands;
}

std::vector<clang::tooling::CompileCommand>
compilation_database::getAllCompileCommands() const
{
    auto commands = base().getAllCompileCommands();

    adjust_compilation_database(commands);

    return commands;
}

std::string compilation_database::guess_language_from_filename(
    const std::string &filename) const
{
    if (util::ends_with(filename, std::string{".c"}))
        return "c";

    return "c++";
}

size_t compilation_database::count_matching_commands(
    const std::vector<std::string> &files) const
{
    if (is_fixed())
        return files.size();

    auto result{0UL};

    for (const auto &f : files) {
        result += getCompileCommands(f).size();
    }

    return result;
}

void compilation_database::adjust_compilation_database(
    std::vector<clang::tooling::CompileCommand> &commands) const
{
#if !defined(_WIN32)
    if (config().query_driver && !config().query_driver().empty()) {
        for (auto &compile_command : commands) {
            auto argv0 = config().query_driver() == "."
                ? compile_command.CommandLine.at(0)
                : config().query_driver();

            util::query_driver_output_extractor extractor{
                argv0, guess_language_from_filename(compile_command.Filename)};

            extractor.execute();

            std::vector<std::string> system_header_args;
            for (const auto &path : extractor.system_include_paths()) {
                system_header_args.emplace_back("-isystem");
                system_header_args.emplace_back(path);
            }

            compile_command.CommandLine.insert(
                compile_command.CommandLine.begin() + 1,
                system_header_args.begin(), system_header_args.end());

            if (!extractor.target().empty()) {
                compile_command.CommandLine.insert(
                    compile_command.CommandLine.begin() + 1,
                    fmt::format("--target={}", extractor.target()));
            }
        }
    }
#endif

    if (config().add_compile_flags && !config().add_compile_flags().empty()) {
        for (auto &compile_command : commands) {
            compile_command.CommandLine.insert(
                // Add flags after argv[0]
                compile_command.CommandLine.begin() + 1,
                config().add_compile_flags().begin(),
                config().add_compile_flags().end());
        }
    }

    if (config().remove_compile_flags &&
        !config().remove_compile_flags().empty()) {
        for (auto &compile_command : commands) {
            for (const auto &flag : config().remove_compile_flags()) {
                util::erase_if(compile_command.CommandLine,
                    [&flag](const auto &arg) { return flag == arg; });
            }
        }
    }
}

bool compilation_database::match_filename(
    const clang::tooling::CompileCommand &command,
    const std::string &file) const
{
    auto command_filename = std::filesystem::path{command.Filename};

    if (!command_filename.is_absolute()) {
        command_filename = config().root_directory() / command_filename;
    }

    return (command_filename == file) ||
        (command_filename.lexically_normal().string() == file);
}
} // namespace clanguml::common