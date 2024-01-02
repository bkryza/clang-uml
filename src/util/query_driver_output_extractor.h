/**
 * @file src/util/query_driver_include_extractor.h
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

#include <stdexcept>
#include <string>
#include <vector>

namespace clanguml::util {

/**
 * @brief Executed compiler frontend and extract default system paths
 *
 * This class - inspired by the `clangd` language server - will invoke the
 * provided compiler command and query it for its default system paths,
 * which then will be added to each compile command in the database.
 */
class query_driver_output_extractor {
public:
    /**
     * @brief Constructor.
     *
     * @param command Command to execute the compiler frontend
     * @param language Language name to query for (C or C++)
     */
    query_driver_output_extractor(std::string command, std::string language);

    ~query_driver_output_extractor() = default;

    /**
     * @brief Execute the command and extract compiler flags and include paths
     */
    void execute();

    /**
     * @brief Extract target name from the compiler output
     *
     * @param output Compiler query driver output
     */
    void extract_target(const std::string &output);

    /**
     * @brief Extract system include paths from the compiler output
     *
     * @param output Compiler query driver output
     */
    void extract_system_include_paths(const std::string &output);

    /**
     * @brief Name of the target of the compiler command (e.g. x86_64-linux-gnu)
     *
     * @return Target name
     */
    const std::string &target() const;

    /**
     * @brief Return list of include system paths
     *
     * @return List of include system paths
     */
    const std::vector<std::string> &system_include_paths() const;

private:
    std::string command_;
    std::string language_;
    std::string target_;
    std::vector<std::string> system_include_paths_;
};
} // namespace clanguml::util