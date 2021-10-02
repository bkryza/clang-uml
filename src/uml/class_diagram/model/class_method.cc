/**
 * src/uml/class_diagram/model/class_method.cc
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

#include "class_method.h"

namespace clanguml::class_diagram::model {

class_method::class_method(
    scope_t scope, const std::string &name, const std::string &type)
    : class_element{scope, name, type}
{
}

bool class_method::is_pure_virtual() const { return is_pure_virtual_; }

void class_method::is_pure_virtual(bool is_pure_virtual)
{
    is_pure_virtual_ = is_pure_virtual;
}

bool class_method::is_virtual() const { return is_virtual_; }

void class_method::is_virtual(bool is_virtual) { is_virtual_ = is_virtual; }

bool class_method::is_const() const { return is_const_; }

void class_method::is_const(bool is_const) { is_const_ = is_const; }

bool class_method::is_defaulted() const { return is_defaulted_; }

void class_method::is_defaulted(bool is_defaulted)
{
    is_defaulted_ = is_defaulted;
}

bool class_method::is_static() const { return is_static_; }

void class_method::is_static(bool is_static) { is_static_ = is_static; }

const std::vector<method_parameter> &class_method::parameters() const
{
    return parameters_;
}

void class_method::add_parameter(method_parameter &&parameter)
{
    parameters_.emplace_back(std::move(parameter));
}
}
