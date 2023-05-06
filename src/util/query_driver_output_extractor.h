/**
 * src/util/query_driver_include_extractor.h
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

#include <stdexcept>
#include <string>
#include <vector>

namespace clanguml::util {

class query_driver_no_paths : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class query_driver_output_extractor {
public:
    query_driver_output_extractor(std::string command, std::string language);

    ~query_driver_output_extractor() = default;

    void execute();

    void extract_system_include_paths(const std::string &output);

    const std::vector<std::string> &system_include_paths() const;

private:
    const std::string command_;
    const std::string language_;
    std::vector<std::string> system_include_paths_;
};
} // namespace clanguml::util