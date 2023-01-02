/**
 * src/class_diagram/model/method_parameter.cc
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

#include "method_parameter.h"

#include "util/util.h"

namespace clanguml::class_diagram::model {

void method_parameter::set_type(const std::string &type) { type_ = type; }

std::string method_parameter::type() const { return type_; }

void method_parameter::set_name(const std::string &name) { name_ = name; }

std::string method_parameter::name() const { return name_; }

void method_parameter::set_default_value(const std::string &value)
{
    default_value_ = value;
}

std::string method_parameter::default_value() const { return default_value_; }

std::string method_parameter::to_string(
    const common::model::namespace_ &using_namespace) const
{
    using namespace clanguml::util;
    auto type_ns =
        using_namespace.relative(common::model::namespace_{type()}.to_string());
    if (default_value().empty())
        return fmt::format("{} {}", type_ns, name());

    return fmt::format("{} {} = {}", type_ns, name(), default_value());
}

} // namespace clanguml::class_diagram::model
