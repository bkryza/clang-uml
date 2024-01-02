/**
 * @file src/class_diagram/model/method_parameter.h
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

#include "common/model/decorated_element.h"
#include "common/model/namespace.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

/**
 * @brief Model of a method parameter.
 */
class method_parameter : public common::model::decorated_element {
public:
    method_parameter() = default;

    /**
     * @brief Constructor.
     *
     * @param type Type of the method parameter as string.
     * @param name Name of the method parameter.
     * @param default_value Default value of the parameter or empty.
     */
    method_parameter(
        std::string type, std::string name, std::string default_value = {});

    ~method_parameter() override = default;

    /**
     * @brief Set parameters type.
     *
     * @param type Parameters type as string.
     */
    void set_type(const std::string &type);

    /**
     * @brief Get parameters type.
     *
     * @return Parameters type as string.
     */
    std::string type() const;

    /**
     * @brief Set parameters name.
     *
     * @param type Parameters name.
     */
    void set_name(const std::string &name);

    /**
     * @brief Get parameters name.
     *
     * @return Parameters name.
     */
    std::string name() const;

    /**
     * @brief Set parameters default value.
     *
     * @param type Parameters default value as string.
     */
    void set_default_value(const std::string &value);

    /**
     * @brief Get parameters name.
     *
     * @return Parameters name.
     */
    std::string default_value() const;

    /**
     * @brief Render the method parameter to a string.
     *
     * @param using_namespaces If provided, make all namespaces relative to it.
     * @return String representation of the parameter.
     */
    std::string to_string(
        const common::model::namespace_ &using_namespaces) const;

private:
    std::string type_;
    std::string name_;
    std::string default_value_;
};

} // namespace clanguml::class_diagram::model
