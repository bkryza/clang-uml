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

#include "common/model/enums.h"

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

class diagram_generation_error : public std::runtime_error {
public:
    diagram_generation_error(common::model::diagram_t type, std::string name,
        std::string description)
        : std::runtime_error{description}
        , type_{type}
        , name_{std::move(name)}
    {
        message_ =
            fmt::format("Generation of {} diagram '{}' failed due to: {}",
                diagram_type(), diagram_name(), description);
    }

    const char *what() const noexcept override { return message_.c_str(); }

    const std::string &diagram_name() const noexcept { return name_; }
    common::model::diagram_t diagram_type() const noexcept { return type_; }

private:
    common::model::diagram_t type_;
    std::string name_;

    std::string message_;
};

class empty_diagram_error : public diagram_generation_error {
    using diagram_generation_error::diagram_generation_error;
};

class invalid_sequence_from_condition : public diagram_generation_error {
    using diagram_generation_error::diagram_generation_error;
};

class invalid_sequence_to_condition : public diagram_generation_error {
    using diagram_generation_error::diagram_generation_error;
};

} // namespace clanguml::error
