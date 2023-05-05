/**
 * src/common/compilation_database.cc
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

#include "compilation_database.h"

namespace clanguml::common {

std::unique_ptr<compilation_database>
compilation_database::auto_detect_from_directory(
    const clanguml::config::config &cfg)
{
    std::string error_message;
    auto res = clang::tooling::CompilationDatabase::autoDetectFromDirectory(
        cfg.compilation_database_dir(), error_message);

    if (!error_message.empty())
        throw compilation_database_error(error_message);

    return std::make_unique<compilation_database>(std::move(res), cfg);
}

compilation_database::compilation_database(
    std::unique_ptr<clang::tooling::CompilationDatabase> base,
    const clanguml::config::config &cfg)
    : base_{std::move(base)}
    , config_{cfg}
{
}

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

void compilation_database::adjust_compilation_database(
    std::vector<clang::tooling::CompileCommand> &commands) const
{
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

} // namespace clanguml::common