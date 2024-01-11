/**
 * @file src/common/types.cc
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

#include "types.h"

namespace clanguml::common {

std::string to_string(const std::string &s) { return s; }

std::string to_string(const string_or_regex &sr) { return sr.to_string(); }

std::string to_string(const generator_type_t type)
{
    switch (type) {
    case generator_type_t::plantuml:
        return "plantuml";
    case generator_type_t::mermaid:
        return "mermaid";
    case generator_type_t::json:
        return "json";
    default:
        return "<unknown>";
    }
}
} // namespace clanguml::common
