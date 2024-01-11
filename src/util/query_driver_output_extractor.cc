/**
 * @file src/util/query_driver_include_extractor.cc
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

#include "query_driver_output_extractor.h"

#include "error.h"
#include "util.h"

#include <sstream>
#include <string>

namespace clanguml::util {

query_driver_output_extractor::query_driver_output_extractor(
    std::string command, std::string language)
    : command_{std::move(command)}
    , language_{std::move(language)}
{
}

void query_driver_output_extractor::execute()
{
    auto cmd =
        fmt::format("{} -E -v -x {} /dev/null 2>&1", command_, language_);

    LOG_DBG("Executing query driver command: {}", cmd);

    auto driver_output = get_process_output(cmd);

    system_include_paths_.clear();
    extract_system_include_paths(driver_output);
    extract_target(driver_output);

    if (system_include_paths_.empty()) {
        throw error::query_driver_no_paths(fmt::format(
            "Compiler driver {} did not report any system include paths "
            "in its output: {}",
            command_, driver_output));
    }

    LOG_DBG("Extracted the following paths from compiler driver: {}",
        fmt::join(system_include_paths_, ","));
}

void query_driver_output_extractor::extract_target(const std::string &output)
{
    std::istringstream f(output);
    std::string line;

    while (std::getline(f, line)) {
        line = trim(line);
        if (util::starts_with(line, std::string{"Target: "})) {
            target_ = line.substr(strlen("Target: "));
            break;
        }
    }
}

void query_driver_output_extractor::extract_system_include_paths(
    const std::string &output)
{
    std::istringstream f(output);
    std::string line;

    bool in_include_paths_range{false};
    while (std::getline(f, line)) {
        line = trim(line);
        if (line == "#include <...> search starts here:") {
            in_include_paths_range = true;
            continue;
        }
        if (line == "End of search list.") {
            break;
        }

        if (in_include_paths_range) {
            system_include_paths_.emplace_back(line);
        }
    }
}

const std::vector<std::string> &
query_driver_output_extractor::system_include_paths() const
{
    return system_include_paths_;
}

const std::string &query_driver_output_extractor::target() const
{
    return target_;
}
} // namespace clanguml::util