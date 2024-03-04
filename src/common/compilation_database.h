/**
 * @file src/common/compilation_database.h
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
#pragma once

#include "common/model/enums.h"
#include "common/model/namespace.h"
#include "common/model/template_parameter.h"
#include "config/config.h"
#include "types.h"
#include "util/error.h"
#include "util/util.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <deque>
#include <filesystem>
#include <string>

namespace clanguml::common {

/**
 * @brief Custom compilation database class
 *
 * This class provides custom specialization of Clang's
 * [CompilationDatabase](https://clang.llvm.org/doxygen/classclang_1_1tooling_1_1CompilationDatabase.html),
 * which provides the possibility of adjusting the compilation flags after
 * they have been loaded from the `compile_commands.json` file.
 *
 * @embed{compilation_database_context_class.svg}
 */
class compilation_database : public clang::tooling::CompilationDatabase {
public:
    compilation_database(
        std::unique_ptr<clang::tooling::CompilationDatabase> base,
        const clanguml::config::config &cfg);

    ~compilation_database() override = default;

    /**
     * Loads the compilation database from directory specified on command
     * line or in the configuration file.
     *
     * @param cfg Reference to config instance
     * @return Instance of compilation_database.
     */
    static std::unique_ptr<compilation_database> auto_detect_from_directory(
        const clanguml::config::config &cfg);

    /**
     * Retrieves and adjusts compilation commands from the database, for
     * a given translation unit.
     *
     * @return List of adjusted compile commands.
     */
    std::vector<clang::tooling::CompileCommand> getCompileCommands(
        clang::StringRef FilePath) const override;

    /**
     * Returns all files in the database.
     *
     * @return List of all files in compilation database.
     */
    std::vector<std::string> getAllFiles() const override;

    /**
     * Retrieves and adjusts all compilation commands from the database.
     *
     * @return List of adjusted compile commands.
     */
    std::vector<clang::tooling::CompileCommand>
    getAllCompileCommands() const override;

    /**
     * Returns reference to clanguml's config instance.
     *
     * @return Reference to config instance.
     */
    const clanguml::config::config &config() const;

    /**
     * Returns reference to CompilationDatabase as was loaded from file.
     *
     * @return Reference to CompilationDatabase.
     */
    const clang::tooling::CompilationDatabase &base() const;

    std::string guess_language_from_filename(const std::string &filename) const;

    long count_matching_commands(const std::vector<std::string> &files) const;

private:
    void adjust_compilation_database(
        std::vector<clang::tooling::CompileCommand> &commands) const;

    /*!
     * Pointer to the Clang's original compilation database.
     *
     * Actual instance of the compilation database is stored in here.
     * The inheritance is just to keep the interface.
     */
    std::unique_ptr<clang::tooling::CompilationDatabase> base_;

    /*!
     * Reference to the instance of clanguml config.
     */
    const clanguml::config::config &config_;
};

using compilation_database_ptr = std::unique_ptr<compilation_database>;

} // namespace clanguml::common