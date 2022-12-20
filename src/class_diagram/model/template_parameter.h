/**
 * src/class_diagram/model/template_parameter.h
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

#include "common/model/enums.h"
#include "common/model/namespace.h"

#include <optional>
#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

/// @brief Represents template parameter or template argument
///
/// This class can represent both template parameter and template arguments,
/// including variadic parameters and instantiations with
/// nested templates
class template_parameter {
public:
    template_parameter(const std::string &type = "",
        const std::string &name = "", const std::string &default_value = "",
        bool is_variadic = false);

    template_parameter(const template_parameter &right) = default;

    void set_type(const std::string &type);
    std::string type() const;

    void set_id(const int64_t id) { id_ = id; }
    std::optional<int64_t> id() const { return id_; }

    void set_name(const std::string &name);
    std::string name() const;

    void set_default_value(const std::string &value);
    std::string default_value() const;

    void is_variadic(bool is_variadic) noexcept;
    bool is_variadic() const noexcept;

    void is_pack(bool is_pack) noexcept;
    bool is_pack() const noexcept;

    bool is_specialization_of(const template_parameter &ct) const;

    friend bool operator==(
        const template_parameter &l, const template_parameter &r);
    friend bool operator!=(
        const template_parameter &l, const template_parameter &r);

    bool is_template_parameter() const { return is_template_parameter_; }

    void is_template_parameter(bool is_template_parameter)
    {
        is_template_parameter_ = is_template_parameter;
    }

    bool is_template_template_parameter() const
    {
        return is_template_template_parameter_;
    }

    void is_template_template_parameter(bool is_template_template_parameter)
    {
        is_template_template_parameter_ = is_template_template_parameter;
    }

    std::string to_string(
        const clanguml::common::model::namespace_ &using_namespace,
        bool relative) const;

    void add_template_param(template_parameter &&ct);

    void add_template_param(const template_parameter &ct);

    const std::vector<template_parameter> &template_params() const;

    void clear_params() { template_params_.clear(); }

    bool find_nested_relationships(
        std::vector<std::pair<int64_t, common::model::relationship_t>>
            &nested_relationships,
        common::model::relationship_t hint,
        std::function<bool(const std::string &full_name)> should_include) const;

private:
    /// Represents the type of non-type template parameters
    /// e.g. 'int'
    std::string type_;

    /// The name of the parameter (e.g. 'T' or 'N')
    std::string name_;

    /// Default value of the template parameter
    std::string default_value_;

    /// Whether the template parameter is a regular template parameter
    /// When false, it is a non-type template parameter
    bool is_template_parameter_{false};

    /// Whether the template parameter is a template template parameter
    /// Can only be true when is_template_parameter_ is true
    bool is_template_template_parameter_{false};

    /// Whether the template parameter is variadic
    bool is_variadic_{false};

    /// Whether the argument specializes argument pack from parent template
    bool is_pack_{false};

    // Nested template parameters
    std::vector<template_parameter> template_params_;

    std::optional<int64_t> id_;
};
} // namespace clanguml::class_diagram::model
