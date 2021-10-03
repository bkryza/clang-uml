/**
 * src/class_diagram/model/class_template.cc
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

#include "class_template.h"

namespace clanguml::class_diagram::model {

class_template::class_template(const std::string &type, const std::string &name,
    const std::string &default_value, bool is_variadic)
    : type_{type}
    , name_{name}
    , default_value_{default_value}
    , is_variadic_{is_variadic}
{
    if (is_variadic)
        name_ = name_ + "...";
}

void class_template::set_type(const std::string &type) { type_ = type; }

std::string class_template::type() const { return type_; }

void class_template::set_name(const std::string &name) { name_ = name; }

std::string class_template::name() const { return name_; }

void class_template::set_default_value(const std::string &value)
{
    default_value_ = value;
}

std::string class_template::default_value() const { return default_value_; }

void class_template::is_variadic(bool is_variadic) noexcept
{
    is_variadic_ = is_variadic;
}

bool class_template::is_variadic() const noexcept { return is_variadic_; }

bool operator==(const class_template &l, const class_template &r)
{
    return (l.name() == r.name()) && (l.type() == r.type());
}
}
