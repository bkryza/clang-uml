/**
 * @file src/common/generators/json/generator.cc
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

#include "generator.h"

namespace clanguml::common::generators::graphml {

std::string render_name(std::string name)
{
    util::replace_all(name, "##", "::");

    return name;
}

std::string to_string(const property_type t)
{
    switch (t) {
    case property_type::kBoolean:
        return "boolean";
    case property_type::kInt:
        return "int";
    case property_type::kLong:
        return "long";
    case property_type::kFloat:
        return "float";
    case property_type::kDouble:
        return "double";
    case property_type::kString:
        return "string";
    default:
        assert(false);
        return "";
    }
}

} // namespace clanguml::common::generators::graphml
