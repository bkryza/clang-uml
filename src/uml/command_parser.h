/**
 * src/uml/command_parser.h
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

#include "config/config.h"
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml {
namespace command_parser {

struct command {
    virtual ~command() = default;
    static std::shared_ptr<command> from_string(std::string_view c);
};

struct note : public command {
    std::string position;
    std::string text;

    static std::shared_ptr<command> from_string(std::string_view c);
};

struct style : public command {
    std::string spec;
    static std::shared_ptr<command> from_string(std::string_view c);
};

struct aggregation : public command {
    std::string multiplicity;
    static std::shared_ptr<command> from_string(std::string_view c);
};

std::vector<std::shared_ptr<command>> parse(std::string documentation_block);

} // namespace command_parser
} // namespace clanguml

