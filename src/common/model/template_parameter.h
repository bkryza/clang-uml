/**
 * src/common/model/template_parameter.h
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

#include "common/model/enums.h"
#include "common/model/namespace.h"

#include <optional>
#include <string>
#include <vector>

namespace clanguml::common::model {

enum class template_parameter_kind_t {
    template_type,
    template_template_type,
    non_type_template,
    argument, // a.k.a. type parameter specialization
    concept_constraint
};

std::string to_string(template_parameter_kind_t k);

/// @brief Represents template parameter, template arguments or concept
///        constraints
///
/// This class can represent both template parameter and template arguments,
/// including variadic parameters and instantiations with
/// nested templates
class template_parameter {
public:
    static template_parameter make_template_type(const std::string &name,
        const std::optional<std::string> &default_value = {},
        bool is_variadic = false)
    {
        template_parameter p;
        p.set_kind(template_parameter_kind_t::template_type);
        p.set_name(name);
        p.is_variadic(is_variadic);
        p.is_template_parameter(true);
        if (default_value)
            p.set_default_value(default_value.value());
        return p;
    }

    static template_parameter make_template_template_type(
        const std::string &name,
        const std::optional<std::string> &default_value = {},
        bool is_variadic = false)
    {
        template_parameter p;
        p.set_kind(template_parameter_kind_t::template_template_type);
        p.set_name(name + "<>");
        if (default_value)
            p.set_default_value(default_value.value());
        p.is_variadic(is_variadic);
        return p;
    }

    static template_parameter make_non_type_template(const std::string &type,
        const std::optional<std::string> &name,
        const std::optional<std::string> &default_value = {},
        bool is_variadic = false)
    {
        template_parameter p;
        p.set_kind(template_parameter_kind_t::non_type_template);
        p.set_type(type);
        if (name)
            p.set_name(name.value());
        if (default_value)
            p.set_default_value(default_value.value());
        p.is_variadic(is_variadic);
        return p;
    }

    static template_parameter make_argument(const std::string &type,
        const std::optional<std::string> &default_value = {})
    {
        template_parameter p;
        p.set_kind(template_parameter_kind_t::argument);
        p.set_type(type);
        if (default_value)
            p.set_default_value(default_value.value());
        return p;
    }

    static template_parameter make_unexposed_argument(const std::string &type,
        const std::optional<std::string> &default_value = {})
    {
        template_parameter p = make_argument(type, default_value);
        p.set_unexposed(true);
        return p;
    }

    void set_type(const std::string &type);
    std::optional<std::string> type() const;

    void set_id(const int64_t id) { id_ = id; }
    const std::optional<int64_t> &id() const { return id_; }

    void set_name(const std::string &name);
    std::optional<std::string> name() const;

    void set_default_value(const std::string &value);
    const std::optional<std::string> &default_value() const;

    void is_variadic(bool is_variadic) noexcept;
    bool is_variadic() const noexcept;

    int calculate_specialization_match(
        const template_parameter &base_template_parameter) const;

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
        const std::function<bool(const std::string &full_name)> &should_include)
        const;

    void set_concept_constraint(std::string constraint);

    const std::optional<std::string> &concept_constraint() const;

    template_parameter_kind_t kind() const { return kind_; }

    void set_kind(template_parameter_kind_t kind) { kind_ = kind; }

    bool is_unexposed() const { return is_unexposed_; }

    void set_unexposed(bool unexposed) { is_unexposed_ = unexposed; }

    void set_function_template(bool ft) { is_function_template_ = ft; }

    bool is_function_template() const { return is_function_template_; }

private:
    template_parameter() = default;

    template_parameter_kind_t kind_{template_parameter_kind_t::template_type};

    /// Represents the type of non-type template parameters
    /// e.g. 'int' or type of template arguments
    std::optional<std::string> type_;

    /// The name of the parameter (e.g. 'T' or 'N')
    std::optional<std::string> name_;

    /// Default value of the template parameter
    std::optional<std::string> default_value_;

    /// Whether the template parameter is a regular template parameter
    /// When false, it is a non-type template parameter
    bool is_template_parameter_{false};

    /// Whether the template parameter is a template template parameter
    /// Can only be true when is_template_parameter_ is true
    bool is_template_template_parameter_{false};

    /// Whether the template parameter is variadic
    bool is_variadic_{false};

    bool is_function_template_{false};

    /// Stores optional fully qualified name of constraint for this template
    /// parameter
    std::optional<std::string> concept_constraint_;

    // Nested template parameters
    // If this is a function template, the first element is the return type
    std::vector<template_parameter> template_params_;

    std::optional<int64_t> id_;

    bool is_unexposed_{false};
};

int calculate_template_params_specialization_match(
    const std::vector<template_parameter> &specialization,
    const std::vector<template_parameter> &base_template);

} // namespace clanguml::common::model
