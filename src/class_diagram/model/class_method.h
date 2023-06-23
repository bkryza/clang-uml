/**
 * @file src/class_diagram/model/class_method.h
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
#pragma once

#include "class_element.h"
#include "common/model/template_parameter.h"
#include "common/model/template_trait.h"
#include "method_parameter.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

using clanguml::common::model::template_trait;

class class_method : public class_element, public template_trait {
public:
    class_method(common::model::access_t access, const std::string &name,
        const std::string &type);

    ~class_method() override = default;

    bool is_pure_virtual() const;
    void is_pure_virtual(bool is_pure_virtual);

    bool is_virtual() const;
    void is_virtual(bool is_virtual);

    bool is_const() const;
    void is_const(bool is_const);

    bool is_defaulted() const;
    void is_defaulted(bool is_defaulted);

    bool is_deleted() const;
    void is_deleted(bool is_deleted);

    bool is_static() const;
    void is_static(bool is_static);

    bool is_constexpr() const;
    void is_constexpr(bool is_constexpr);

    bool is_consteval() const;
    void is_consteval(bool is_consteval);

    bool is_noexcept() const;
    void is_noexcept(bool is_noexcept);

    bool is_constructor() const;
    void is_constructor(bool is_constructor);

    bool is_destructor() const;
    void is_destructor(bool is_destructor);

    bool is_move_assignment() const;
    void is_move_assignment(bool is_move_assignment);

    bool is_copy_assignment() const;
    void is_copy_assignment(bool is_copy_assignment);

    bool is_operator() const;
    void is_operator(bool is_operator);

    const std::vector<method_parameter> &parameters() const;
    void add_parameter(method_parameter &&parameter);

private:
    std::vector<method_parameter> parameters_;

    bool is_pure_virtual_{false};
    bool is_virtual_{false};
    bool is_const_{false};
    bool is_defaulted_{false};
    bool is_deleted_{false};
    bool is_static_{false};
    bool is_noexcept_{false};
    bool is_constexpr_{false};
    bool is_consteval_{false};
    bool is_constructor_{false};
    bool is_destructor_{false};
    bool is_move_assignment_{false};
    bool is_copy_assignment_{false};
    bool is_operator_{false};
};
} // namespace clanguml::class_diagram::model
