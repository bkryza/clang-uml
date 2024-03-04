/**
 * @file src/util/error.h
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

namespace clanguml::error {

class query_driver_no_paths : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct uml_alias_missing : public virtual std::runtime_error {
    uml_alias_missing(const std::string &message)
        : std::runtime_error(message)
    {
    }
};

class compilation_database_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class empty_diagram_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

} // namespace clanguml::error
