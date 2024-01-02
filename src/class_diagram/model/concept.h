/**
 * @file src/class_diagram/model/concept.h
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

#include "class_diagram/model/method_parameter.h"
#include "common/model/element.h"
#include "common/model/stylable_element.h"
#include "common/model/template_parameter.h"
#include "common/model/template_trait.h"
#include "common/types.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

/**
 * @brief Model of C++ concept.
 */
class concept_ : public common::model::element,
                 public common::model::stylable_element,
                 public common::model::template_trait {
public:
    concept_(const common::model::namespace_ &using_namespace);

    concept_(const concept_ &) = delete;
    concept_(concept_ &&) noexcept = default;
    concept_ &operator=(const concept_ &) = delete;
    concept_ &operator=(concept_ &&) = delete;

    /**
     * @brief Get the elements type name.
     *
     * @return 'concept'
     */
    std::string type_name() const override { return "concept"; }

    friend bool operator==(const concept_ &l, const concept_ &r);

    std::string full_name(bool relative = true) const override;

    std::string full_name_no_ns() const override;

    /**
     * @brief Add concept parameter
     *
     * Concept class for convenience uses the same method parameter model
     * as regular methods and functions.
     *
     * @param mp Concept parameter
     */
    void add_parameter(const method_parameter &mp);

    /**
     * @brief Get concepts requires expression parameters
     *
     * @return List of concept requires expression parameters
     */
    const std::vector<method_parameter> &requires_parameters() const;

    /**
     * @brief Add a concept statement
     *
     * @param stmt Concept statement
     */
    void add_statement(std::string stmt);

    /**
     * @brief Get the concepts requires statements
     *
     * @return List of concepts requires statements
     */
    const std::vector<std::string> &requires_statements() const;

private:
    std::vector<std::string> requires_expression_;

    std::vector<method_parameter> requires_parameters_;

    std::vector<std::string> requires_statements_;
};
} // namespace clanguml::class_diagram::model