/**
 * src/common/model/template_parameter.cc
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

#include "template_parameter.h"
#include "common/model/enums.h"
#include <common/model/namespace.h>

#include <utility>

namespace clanguml::common::model {
std::string to_string(template_parameter_kind_t k)
{
    switch (k) {
    case template_parameter_kind_t::template_type:
        return "template_type";
    case template_parameter_kind_t::template_template_type:
        return "template_template_type";
    case template_parameter_kind_t::non_type_template:
        return "non_type_template";
    case template_parameter_kind_t::argument:
        return "argument";
    case template_parameter_kind_t::concept_constraint:
        return "concept_constraint";
    default:
        assert(false);
        return "";
    }
}

void template_parameter::set_type(const std::string &type)
{
    assert(kind_ != template_parameter_kind_t::template_type);

    if (util::ends_with(type, std::string{"..."})) {
        type_ = type.substr(0, type.size() - 3);
        is_variadic_ = true;
    }
    else
        type_ = type;
}

std::optional<std::string> template_parameter::type() const
{
    if (!type_)
        return {};

    if (is_variadic_)
        return type_.value() + "...";

    return type_;
}

void template_parameter::set_name(const std::string &name)
{
    assert(kind_ != template_parameter_kind_t::argument);

    if (name.empty())
        return;

    if (util::ends_with(name, std::string{"..."})) {
        name_ = name.substr(0, name.size() - 3);
        is_variadic_ = true;
    }
    else
        name_ = name;
}

std::optional<std::string> template_parameter::name() const
{
    if (!name_)
        return {};

    if (is_variadic_ && (kind_ != template_parameter_kind_t::non_type_template))
        return name_.value() + "...";

    return name_;
}

void template_parameter::set_default_value(const std::string &value)
{
    default_value_ = value;
}

const std::optional<std::string> &template_parameter::default_value() const
{
    return default_value_;
}

void template_parameter::is_variadic(bool is_variadic) noexcept
{
    is_variadic_ = is_variadic;
}

bool template_parameter::is_variadic() const noexcept { return is_variadic_; }

bool template_parameter::is_specialization_of(
    const template_parameter &ct) const
{
    return (ct.is_template_parameter() ||
               ct.is_template_template_parameter()) &&
        !is_template_parameter();
}

void template_parameter::add_template_param(template_parameter &&ct)
{
    template_params_.emplace_back(std::move(ct));
}

void template_parameter::add_template_param(const template_parameter &ct)
{
    template_params_.push_back(ct);
}

const std::vector<template_parameter> &
template_parameter::template_params() const
{
    return template_params_;
}

bool operator==(const template_parameter &l, const template_parameter &r)
{
    bool res{false};

    if (l.is_template_parameter() != r.is_template_parameter())
        return res;

    if (l.is_template_parameter()) {
        // If this is a template parameter (e.g. 'typename T' or 'typename U'
        // we don't actually care what it is called
        res = (l.is_variadic() == r.is_variadic()) &&
            (l.default_value() == r.default_value());
    }
    else
        res = (l.name() == r.name()) && (l.type() == r.type()) &&
            (l.default_value() == r.default_value());

    return res && (l.template_params_ == r.template_params_);
}

bool operator!=(const template_parameter &l, const template_parameter &r)
{
    return !(l == r);
}

std::string template_parameter::to_string(
    const clanguml::common::model::namespace_ &using_namespace,
    bool relative) const
{
    using clanguml::common::model::namespace_;

    assert(!(type().has_value() && concept_constraint().has_value()));

    std::string res;
    const auto maybe_type = type();
    if (maybe_type) {
        if (!relative)
            res += namespace_{*maybe_type}.to_string();
        else
            res += namespace_{*maybe_type}
                       .relative_to(using_namespace)
                       .to_string();
    }

    const auto &maybe_concept_constraint = concept_constraint();

    if (maybe_concept_constraint) {
        if (!relative)
            res += namespace_{maybe_concept_constraint.value()}.to_string();
        else
            res += namespace_{maybe_concept_constraint.value()}
                       .relative_to(using_namespace)
                       .to_string();
    }

    const auto maybe_name = name();

    if (maybe_name) {
        if ((maybe_type && !maybe_type.value().empty()) ||
            maybe_concept_constraint)
            res += " ";

        if (!relative)
            res += namespace_{*maybe_name}.to_string();
        else
            res += namespace_{*maybe_name}
                       .relative_to(using_namespace)
                       .to_string();
    }

    // Render nested template params
    if (!template_params_.empty()) {
        std::vector<std::string> params;
        params.reserve(template_params_.size());
        for (const auto &template_param : template_params_) {
            params.push_back(
                template_param.to_string(using_namespace, relative));
        }

        res += fmt::format("<{}>", fmt::join(params, ","));
    }

    const auto &maybe_default_value = default_value();
    if (maybe_default_value) {
        res += "=";
        res += maybe_default_value.value();
    }

    return res;
}

bool template_parameter::find_nested_relationships(
    std::vector<std::pair<int64_t, common::model::relationship_t>>
        &nested_relationships,
    common::model::relationship_t hint,
    const std::function<bool(const std::string &full_name)> &should_include)
    const
{
    bool added_aggregation_relationship{false};

    // If this type argument should be included in the relationship
    // just add it and skip recursion (e.g. this is a user defined type)
    const auto maybe_type = type();
    if (maybe_type && should_include(maybe_type.value())) {
        const auto maybe_id = id();
        if (maybe_id) {
            nested_relationships.emplace_back(maybe_id.value(), hint);
            added_aggregation_relationship =
                (hint == common::model::relationship_t::kAggregation);
        }
    }
    // Otherwise (e.g. this is a std::shared_ptr) and we're actually
    // interested what is stored inside it
    else {
        for (const auto &template_argument : template_params()) {

            const auto maybe_id = template_argument.id();
            const auto maybe_arg_type = template_argument.type();

            if (maybe_id && maybe_arg_type && should_include(*maybe_arg_type)) {

                nested_relationships.emplace_back(maybe_id.value(), hint);

                added_aggregation_relationship =
                    (hint == common::model::relationship_t::kAggregation);
            }
            else {
                added_aggregation_relationship =
                    template_argument.find_nested_relationships(
                        nested_relationships, hint, should_include);
            }
        }
    }

    return added_aggregation_relationship;
}

void template_parameter::set_concept_constraint(std::string constraint)
{
    concept_constraint_ = std::move(constraint);
}

const std::optional<std::string> &template_parameter::concept_constraint() const
{
    return concept_constraint_;
}

} // namespace clanguml::common::model
