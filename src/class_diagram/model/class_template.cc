/**
 * src/class_diagram/model/class_template.cc
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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
#include <common/model/namespace.h>
#include <fmt/format.h>

namespace clanguml::class_diagram::model {

class_template::class_template(const std::string &type, const std::string &name,
    const std::string &default_value, bool is_variadic)
    : name_{name}
    , default_value_{default_value}
    , is_variadic_{is_variadic}
{
    set_type(type);
    if (is_variadic)
        name_ = name_ + "...";
}

void class_template::set_type(const std::string &type) {
    type_ = type;
    // TODO: Add a configurable mapping for simplifying non-interesting
    //       std templates
    util::replace_all(type_, "std::basic_string<char>", "std::string");
}

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

bool operator!=(const class_template &l, const class_template &r)
{
    return !(l == r);
}

std::string class_template::to_string(
    const clanguml::common::model::namespace_ &using_namespace) const
{
    using clanguml::common::model::namespace_;

    std::string res;
    if (!type().empty())
        res += namespace_{type()}.relative_to(using_namespace).to_string();

    // Render nested template params
    if (!template_params_.empty()) {
        std::vector<std::string> params;
        for (const auto &template_param : template_params_) {
            params.push_back(template_param.to_string(using_namespace));
        }

        res += fmt::format("<{}>", fmt::join(params, ","));
    }

    if (!name().empty()) {
        if (!type().empty())
            res += " ";
        res += namespace_{name()}.relative_to(using_namespace).to_string();
    }

    if (!default_value().empty()) {
        res += "=";
        res += default_value();
    }

    return res;
}

}
