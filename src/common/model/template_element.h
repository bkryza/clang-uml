/**
 * @file src/common/model/template_element.h
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

#include "element.h"
#include "template_trait.h"

namespace clanguml::common::model {

/**
 * @brief Base class for any element qualified by namespace.
 */
class template_element : public element, public template_trait {
public:
    using element::element;

    ~template_element() override = default;

    /**
     * Whether or not the class is a template.
     *
     * @return True, if the class is a template.
     */
    bool is_template() const;

    /**
     * Set, whether the class is a template.
     *
     * @param is_struct True, if the class is a template.
     */
    void is_template(bool is_template);

    /**
     * @brief Calculate template specialization match with other class.
     *
     * This method is a wrapper over
     * @ref template_trait::calculate_template_specialization_match()
     *
     * @param other
     * @return
     */
    int calculate_template_specialization_match(
        const template_element &other) const;

    /**
     * Whether, a template specialization has already been found for this class.
     * @return True, if a template specialization has already been found.
     */
    bool template_specialization_found() const;

    /**
     * Set, whether a template specialization has already been found for this
     * class.
     *
     * @param found True, if a template specialization has already been found.
     */
    void template_specialization_found(bool found);

private:
    bool template_specialization_found_{false};
    bool is_template_{false};
};

} // namespace clanguml::common::model