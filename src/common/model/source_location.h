/**
 * src/common/model/source_location.h
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

#include <string>
#include <utility>

namespace clanguml::common::model {

class source_location {
public:
    source_location() = default;

    source_location(std::string f, unsigned int l)
        : file_{std::move(f)}
        , line_{l}
    {
    }

    const std::string &file() const { return file_; }

    void set_file(const std::string &file) { file_ = file; }

    const std::string &file_relative() const { return file_relative_; }

    void set_file_relative(const std::string &file) { file_relative_ = file; }

    unsigned int line() const { return line_; }

    void set_line(const unsigned line) { line_ = line; }

    unsigned int location_id() const { return hash_; }

    void set_location_id(unsigned int h) { hash_ = h; }

private:
    std::string file_;
    std::string file_relative_;
    unsigned int line_{0};
    unsigned int hash_{0};
};
} // namespace clanguml::common::model