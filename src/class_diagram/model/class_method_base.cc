/**
 * @file src/class_diagram/model/class_method_base.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include "class_method_base.h"

namespace clanguml::class_diagram::model {
class_method_base::class_method_base(common::model::access_t access,
    const std::string &name, const std::string &type)
    : class_element{access, name, type}
{
}

std::string class_method_base::display_name() const { return display_name_; }

void class_method_base::set_display_name(const std::string &display_name)
{
    display_name_ = display_name;
}

bool class_method_base::is_static() const { return is_static_; }

void class_method_base::is_static(bool is_static) { is_static_ = is_static; }

const std::vector<method_parameter> &class_method_base::parameters() const
{
    return parameters_;
}

void class_method_base::add_parameter(method_parameter &&parameter)
{
    parameters_.emplace_back(std::move(parameter));
}
} // namespace clanguml::class_diagram::model