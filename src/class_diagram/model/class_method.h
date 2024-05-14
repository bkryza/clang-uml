/**
 * @file src/class_diagram/model/class_method.h
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
#pragma once

#include "class_element.h"
#include "common/model/template_parameter.h"
#include "common/model/template_trait.h"
#include "method_parameter.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

using clanguml::common::model::template_trait;

/**
 * @brief Class method model.
 */
class class_method : public class_element, public template_trait {
public:
    /**
     * @brief Constructor.
     *
     * @param access Methods access scope (e.g. public)
     * @param name Methods name.
     * @param type Methods return type as string.
     */
    class_method(common::model::access_t access, const std::string &name,
        const std::string &type);

    ~class_method() override = default;

    void update(const common::model::namespace_ &un);

    /**
     * @brief Method name including template parameters/arguments if any
     *
     * @return String representation of the methods display name
     */
    std::string display_name() const;

    void set_display_name(const std::string &display_name);

    /**
     * @brief Whether the method is pure virtual.
     *
     * @return True, if the method is pure virtual
     */
    bool is_pure_virtual() const;
    void is_pure_virtual(bool is_pure_virtual);

    /**
     * @brief Whether the method is virtual.
     *
     * @return True, if the method is virtual
     */
    bool is_virtual() const;

    /**
     * @brief Set whether the method is virtual.
     *
     * @param is_virtual True, if the method is virtual
     */
    void is_virtual(bool is_virtual);

    /**
     * @brief Whether the method is const.
     *
     * @return True, if the method is const
     */
    bool is_const() const;

    /**
     * @brief Set whether the method is const.
     *
     * @param is_const True, if the method is const
     */
    void is_const(bool is_const);

    /**
     * @brief Whether the method is defaulted.
     *
     * @return True, if the method is defaulted
     */
    bool is_defaulted() const;

    /**
     * @brief Set whether the method is defaulted.
     *
     * @param is_defaulted True, if the method is defaulted
     */
    void is_defaulted(bool is_defaulted);

    /**
     * @brief Whether the method is deleted.
     *
     * @return True, if the method is deleted
     */
    bool is_deleted() const;

    /**
     * @brief Set whether the method is deleted.
     *
     * @param is_deleted True, if the method is deleted
     */
    void is_deleted(bool is_deleted);

    /**
     * @brief Whether the method is static.
     *
     * @return True, if the method is static
     */
    bool is_static() const;

    /**
     * @brief Set whether the method is static.
     *
     * @param is_static True, if the method is static
     */
    void is_static(bool is_static);

    /**
     * @brief Whether the method is constexpr.
     *
     * @return True, if the method is constexpr
     */
    bool is_constexpr() const;

    /**
     * @brief Set whether the method is constexpr.
     *
     * @param is_constexpr True, if the method is constexpr
     */
    void is_constexpr(bool is_constexpr);

    /**
     * @brief Whether the method is consteval.
     *
     * @return True, if the method is consteval
     */
    bool is_consteval() const;

    /**
     * @brief Set whether the method is consteval.
     *
     * @param is_consteval True, if the method is consteval
     */
    void is_consteval(bool is_consteval);

    /**
     * @brief Whether the method is a C++20 coroutine.
     *
     * @return True, if the method is a coroutine
     */
    bool is_coroutine() const;

    /**
     * @brief Set whether the method is a C++20 coroutine.
     *
     * @param is_coroutine True, if the method is a coroutine
     */
    void is_coroutine(bool is_coroutine);

    /**
     * @brief Whether the method is noexcept.
     *
     * @return True, if the method is noexcept
     */
    bool is_noexcept() const;

    /**
     * @brief Set whether the method is noexcept.
     *
     * @param is_noexcept True, if the method is noexcept
     */
    void is_noexcept(bool is_noexcept);

    /**
     * @brief Whether the method is a constructor.
     *
     * @return True, if the method is a constructor
     */
    bool is_constructor() const;

    /**
     * @brief Set whether the method is a constructor.
     *
     * @param is_constructor True, if the method is a constructor
     */
    void is_constructor(bool is_constructor);

    /**
     * @brief Whether the method is a destructor.
     *
     * @return True, if the method is a destructor
     */
    bool is_destructor() const;

    /**
     * @brief Set whether the method is a destructor.
     *
     * @param is_destructor True, if the method is a destructor
     */
    void is_destructor(bool is_destructor);

    /**
     * @brief Whether the method is move assignment.
     *
     * @return True, if the method is move assignment
     */
    bool is_move_assignment() const;

    /**
     * @brief Set whether the method is a move assignment.
     *
     * @param is_move_assignment True, if the method is a move assignment
     */
    void is_move_assignment(bool is_move_assignment);

    /**
     * @brief Whether the method is copy assignment.
     *
     * @return True, if the method is copy assignment
     */
    bool is_copy_assignment() const;

    /**
     * @brief Set whether the method is a copy assignment.
     *
     * @param is_copy_assignment True, if the method is a copy assignment
     */
    void is_copy_assignment(bool is_copy_assignment);

    /**
     * @brief Whether the method is an operator.
     *
     * @return True, if the method is an operator
     */
    bool is_operator() const;

    /**
     * @brief Set whether the method is an operator.
     *
     * @param is_copy_assignment True, if the method is an operator
     */
    void is_operator(bool is_operator);

    /**
     * @brief Get the method parameters.
     *
     * @return List of methods parameters
     */
    const std::vector<method_parameter> &parameters() const;

    /**
     * @brief Add methods parameter.
     *
     * @param parameter Method parameter.
     */
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
    bool is_coroutine_{false};
    bool is_constructor_{false};
    bool is_destructor_{false};
    bool is_move_assignment_{false};
    bool is_copy_assignment_{false};
    bool is_operator_{false};

    std::string display_name_;
};
} // namespace clanguml::class_diagram::model
