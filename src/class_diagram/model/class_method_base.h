/**
 * @file src/class_diagram/model/class_method_base.h
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
#pragma once

#include "class_element.h"
#include "common/model/template_parameter.h"
#include "common/model/template_trait.h"
#include "method_parameter.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {
/**
 * @brief Class method model.
 */
class class_method_base : public class_element {
public:
    /**
     * @brief Constructor.
     *
     * @param access Methods access scope (e.g. public)
     * @param name Methods name.
     * @param type Methods return type as string.
     */
    class_method_base(common::model::access_t access, const std::string &name,
        const std::string &type);

    ~class_method_base() override = default;

    /**
     * @brief Method name including template parameters/arguments if any
     *
     * @return String representation of the methods display name
     */
    std::string display_name() const;

    void set_display_name(const std::string &display_name);

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

    bool is_static_{false};
    std::string display_name_;
};
} // namespace clanguml::class_diagram::model