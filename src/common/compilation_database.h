/**
 * src/common/compilation_database.h
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
#pragma once

#include "common/model/enums.h"
#include "common/model/namespace.h"
#include "common/model/template_parameter.h"
#include "config/config.h"
#include "types.h"
#include "util/util.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <deque>
#include <filesystem>
#include <string>

namespace clanguml::common {

class compilation_database_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class compilation_database : public clang::tooling::CompilationDatabase {
public:
    compilation_database(
        std::unique_ptr<clang::tooling::CompilationDatabase> base,
        const clanguml::config::config &cfg);

    ~compilation_database() override = default;

    static std::unique_ptr<compilation_database> auto_detect_from_directory(
        const clanguml::config::config &cfg);

    std::vector<clang::tooling::CompileCommand> getCompileCommands(
        clang::StringRef FilePath) const override;

    std::vector<std::string> getAllFiles() const override;

    std::vector<clang::tooling::CompileCommand>
    getAllCompileCommands() const override;

    const clanguml::config::config &config() const;

    const clang::tooling::CompilationDatabase &base() const;

private:
    void adjust_compilation_database(
        std::vector<clang::tooling::CompileCommand> &commands) const;

    // Actual instance of the compilation database is stored in here
    // The inheritance is just to keep the interface
    std::unique_ptr<clang::tooling::CompilationDatabase> base_;

    // Reference to the clang-uml config
    const clanguml::config::config &config_;
};

using compilation_database_ptr = std::unique_ptr<compilation_database>;

} // namespace clanguml::common