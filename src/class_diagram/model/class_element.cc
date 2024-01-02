/**
 * @file src/class_diagram/model/class_element.cc
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

#include "class_element.h"

#include <utility>

namespace clanguml::class_diagram::model {

class_element::class_element(
    common::model::access_t access, std::string name, std::string type)
    : access_{access}
    , name_{std::move(name)}
    , type_{std::move(type)}
{
}

common::model::access_t class_element::access() const { return access_; }

std::string class_element::name() const { return name_; }

void class_element::set_name(const std::string &name) { name_ = name; }

std::string class_element::type() const { return type_; }

void class_element::set_type(const std::string &type) { type_ = type; }

inja::json class_element::context() const
{
    inja::json ctx;
    ctx["name"] = name();
    ctx["type"] = type();
    ctx["access"] = to_string(access());
    return ctx;
}
} // namespace clanguml::class_diagram::model
