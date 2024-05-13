/**
 * @file src/class_diagram/model/class_method.cc
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

#include "class_method.h"

namespace clanguml::class_diagram::model {

class_method::class_method(common::model::access_t access,
    const std::string &name, const std::string &type)
    : class_element{access, name, type}
{
}

void class_method::update(const common::model::namespace_ &un)
{
    if (template_params().empty()) {
        set_display_name(name());
    }
    else {
        std::stringstream template_params_str;
        render_template_params(template_params_str, un, true);
        set_display_name(
            fmt::format("{}{}", name(), template_params_str.str()));
    }
}

std::string class_method::display_name() const { return display_name_; }

void class_method::set_display_name(const std::string &display_name)
{
    display_name_ = display_name;
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

bool class_method::is_deleted() const { return is_deleted_; }

void class_method::is_deleted(bool is_deleted) { is_deleted_ = is_deleted; }

bool class_method::is_static() const { return is_static_; }

void class_method::is_static(bool is_static) { is_static_ = is_static; }

bool class_method::is_constexpr() const { return is_constexpr_; }

void class_method::is_constexpr(bool is_constexpr)
{
    is_constexpr_ = is_constexpr;
}

bool class_method::is_consteval() const { return is_consteval_; }

void class_method::is_consteval(bool is_consteval)
{
    is_consteval_ = is_consteval;
}

bool class_method::is_coroutine() const { return is_coroutine_; }

void class_method::is_coroutine(bool is_coroutine)
{
    is_coroutine_ = is_coroutine;
}

bool class_method::is_noexcept() const { return is_noexcept_; }

void class_method::is_noexcept(bool is_noexcept) { is_noexcept_ = is_noexcept; }

bool class_method::is_constructor() const { return is_constructor_; }

void class_method::is_constructor(bool is_constructor)
{
    is_constructor_ = is_constructor;
}

bool class_method::is_destructor() const { return is_destructor_; }

void class_method::is_destructor(bool is_destructor)
{
    is_destructor_ = is_destructor;
}

bool class_method::is_move_assignment() const { return is_move_assignment_; }

void class_method::is_move_assignment(bool is_move_assignment)
{
    is_move_assignment_ = is_move_assignment;
}

bool class_method::is_copy_assignment() const { return is_copy_assignment_; }

void class_method::is_copy_assignment(bool is_copy_assignment)
{
    is_copy_assignment_ = is_copy_assignment;
}

bool class_method::is_operator() const { return is_operator_; }

void class_method::is_operator(bool is_operator) { is_operator_ = is_operator; }

const std::vector<method_parameter> &class_method::parameters() const
{
    return parameters_;
}

void class_method::add_parameter(method_parameter &&parameter)
{
    parameters_.emplace_back(std::move(parameter));
}

} // namespace clanguml::class_diagram::model
