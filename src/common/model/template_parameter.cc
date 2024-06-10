/**
 * @file src/common/model/template_parameter.cc
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

#include "template_parameter.h"
#include "common/model/enums.h"
#include <common/model/namespace.h>

#include <utility>

namespace clanguml::common::model {

std::string context::to_string() const
{
    std::vector<std::string> cv_qualifiers;
    if (is_const)
        cv_qualifiers.emplace_back("const");
    if (is_volatile)
        cv_qualifiers.emplace_back("volatile");

    auto res = fmt::format("{}", fmt::join(cv_qualifiers, " "));

    if (pr == rpqualifier::kPointer)
        res += "*";
    else if (pr == rpqualifier::kLValueReference)
        res += "&";
    else if (pr == rpqualifier::kRValueReference)
        res += "&&";

    if (is_ref_const)
        res += " const";
    if (is_ref_volatile)
        res += " volatile";

    return res;
}

bool context::operator==(const context &rhs) const
{
    return is_const == rhs.is_const && is_volatile == rhs.is_volatile &&
        is_ref_const == rhs.is_ref_const &&
        is_ref_volatile == rhs.is_ref_volatile && pr == rhs.pr;
}

bool context::operator!=(const context &rhs) const { return !(rhs == *this); }

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

template_parameter template_parameter::make_template_type(
    const std::string &name, const std::optional<std::string> &default_value,
    bool is_variadic)
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

template_parameter template_parameter::make_template_template_type(
    const std::string &name, const std::optional<std::string> &default_value,
    bool is_variadic)
{
    template_parameter p;
    p.set_kind(template_parameter_kind_t::template_template_type);
    p.set_name(name + "<>");
    if (default_value)
        p.set_default_value(default_value.value());
    p.is_variadic(is_variadic);
    return p;
}

template_parameter template_parameter::make_non_type_template(
    const std::string &type, const std::optional<std::string> &name,
    const std::optional<std::string> &default_value, bool is_variadic)
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

template_parameter template_parameter::make_argument(
    const std::string &type, const std::optional<std::string> &default_value)
{
    template_parameter p;
    p.set_kind(template_parameter_kind_t::argument);
    p.set_type(type);
    if (default_value)
        p.set_default_value(default_value.value());
    return p;
}

template_parameter template_parameter::make_unexposed_argument(
    const std::string &type, const std::optional<std::string> &default_value)
{
    template_parameter p = make_argument(type, default_value);
    p.set_unexposed(true);
    return p;
}

bool template_parameter::is_specialization() const
{
    return is_function_template() || is_array() || is_data_pointer() ||
        is_member_pointer() || !deduced_context().empty();
}

bool template_parameter::is_same_specialization(
    const template_parameter &other) const
{
    return is_array() == other.is_array() &&
        is_function_template() == other.is_function_template() &&
        is_data_pointer() == other.is_data_pointer() &&
        is_member_pointer() == other.is_member_pointer();
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

    if (name.empty()) {
        return;
    }

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

    if (kind_ == template_parameter_kind_t::template_type &&
        name_.has_value() && name_.value().empty())
        return "typename";

    if (is_variadic_ && (kind_ != template_parameter_kind_t::non_type_template))
        return name_.value() + "...";

    return name_;
}

void template_parameter::set_default_value(const std::string &value)
{
    assert(kind_ != template_parameter_kind_t::argument);

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

int template_parameter::calculate_specialization_match(
    const template_parameter &base_template_parameter) const
{
    int res{0};

    // If the potential base template has a deduction context (e.g. const&),
    // the specialization must have the same and possibly more
    if (base_template_parameter.is_specialization()) {
        if (!deduced_context().empty() &&
            (base_template_parameter.deduced_context().empty() ||
                !util::starts_with(deduced_context(),
                    base_template_parameter.deduced_context())))
            return 0;

        if (!base_template_parameter.deduced_context().empty() &&
            deduced_context().empty())
            return 0;
    }

    if (is_template_parameter() &&
        base_template_parameter.is_template_parameter() &&
        template_params().empty() &&
        base_template_parameter.template_params().empty() &&
        is_variadic() == is_variadic() &&
        is_function_template() ==
            base_template_parameter.is_function_template() &&
        is_member_pointer() == base_template_parameter.is_member_pointer()) {
        return 1;
    }

    auto maybe_base_template_parameter_type = base_template_parameter.type();
    auto maybe_template_parameter_type = type();

    if (maybe_base_template_parameter_type.has_value() &&
        maybe_template_parameter_type.has_value() &&
        !base_template_parameter.is_template_parameter() &&
        !is_template_parameter()) {
        if (maybe_base_template_parameter_type.value() !=
            maybe_template_parameter_type.value())
            return 0;

        res++;
    }

    if (base_template_parameter.is_array() && !is_array())
        return 0;

    if (base_template_parameter.is_function_template() &&
        !is_function_template())
        return 0;

    if (base_template_parameter.is_member_pointer() && !is_member_pointer())
        return 0;

    if (base_template_parameter.is_data_pointer() && !is_data_pointer())
        return 0;

    if (!base_template_parameter.template_params().empty() &&
        !template_params().empty() &&
        is_same_specialization(base_template_parameter)) {
        auto params_match = calculate_template_params_specialization_match(
            template_params(), base_template_parameter.template_params());

        if (params_match == 0)
            return 0;

        res += params_match;
    }
    else if ((base_template_parameter.is_template_parameter() ||
                 base_template_parameter.is_template_template_parameter()) &&
        !is_template_parameter()) {
        return 1;
    }
    else if (base_template_parameter.is_template_parameter() &&
        base_template_parameter.template_params().empty()) {
        // If the base is a regular template param, only possible with deduced
        // context (deduced context already matches if exists)
        res++;

        if (!deduced_context().empty() &&
            !base_template_parameter.deduced_context().empty() &&
            util::starts_with(
                deduced_context(), base_template_parameter.deduced_context()))
            res += static_cast<int>(
                base_template_parameter.deduced_context().size());
    }

    return res;
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

std::string template_parameter::deduced_context_str() const
{
    std::vector<std::string> deduced_contexts;

    for (const auto &c : deduced_context()) {
        deduced_contexts.push_back(c.to_string());
    }

    return fmt::format("{}", fmt::join(deduced_contexts, " "));
}

std::string template_parameter::to_string(
    const clanguml::common::model::namespace_ &using_namespace, bool relative,
    bool skip_qualifiers) const
{
    if (is_ellipsis())
        return "...";

    using clanguml::common::model::namespace_;

    assert(!(type().has_value() && concept_constraint().has_value()));

    if (is_array()) {
        auto it = template_params_.begin();
        auto element_type = it->to_string(using_namespace, relative);
        std::advance(it, 1);

        std::vector<std::string> dimension_args;
        for (; it != template_params_.end(); it++)
            dimension_args.push_back(it->to_string(using_namespace, relative));

        return fmt::format(
            "{}[{}]", element_type, fmt::join(dimension_args, "]["));
    }

    if (is_function_template()) {
        auto it = template_params_.begin();
        auto return_type = it->to_string(using_namespace, relative);
        std::advance(it, 1);

        std::vector<std::string> function_args;
        for (; it != template_params_.end(); it++)
            function_args.push_back(it->to_string(using_namespace, relative));

        return fmt::format(
            "{}({})", return_type, fmt::join(function_args, ","));
    }

    if (is_data_pointer()) {
        assert(template_params().size() == 2);

        std::string unqualified = fmt::format("{} {}::*",
            template_params().at(0).to_string(using_namespace, relative),
            template_params().at(1).to_string(using_namespace, relative));

        if (skip_qualifiers)
            return unqualified;

        return util::join(" ", unqualified, deduced_context_str());
    }

    if (is_member_pointer()) {
        assert(template_params().size() > 1);

        auto it = template_params().begin();
        auto return_type = it->to_string(using_namespace, relative);
        it++;
        auto class_type = it->to_string(using_namespace, relative);
        it++;
        std::vector<std::string> args;

        for (; it != template_params().end(); it++) {
            args.push_back(it->to_string(using_namespace, relative));
        }

        std::string unqualified = fmt::format(
            "{} ({}::*)({})", return_type, class_type, fmt::join(args, ","));
        if (skip_qualifiers)
            return unqualified;

        return util::join(" ", unqualified, deduced_context_str());
    }

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

    if (!skip_qualifiers)
        res = util::join(" ", res, deduced_context_str());

    const auto &maybe_default_value = default_value();
    if (maybe_default_value) {
        res += "=";
        res += maybe_default_value.value();
    }

    return res;
}

bool template_parameter::find_nested_relationships(
    std::vector<std::pair<eid_t, common::model::relationship_t>>
        &nested_relationships,
    common::model::relationship_t hint,
    const std::function<bool(const std::string &full_name)> &should_include)
    const
{
    bool added_aggregation_relationship{false};

    // If this type argument should be included in the relationship
    // just add it and skip recursion (e.g. this is a user defined type)
    const auto maybe_type = type();

    if (is_function_template())
        hint = common::model::relationship_t::kDependency;

    if (maybe_type && should_include(maybe_type.value())) {
        if (is_association())
            hint = common::model::relationship_t::kAssociation;

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

                if (template_argument.is_association() &&
                    hint == common::model::relationship_t::kAggregation)
                    hint = common::model::relationship_t::kAssociation;

                nested_relationships.emplace_back(maybe_id.value(), hint);

                added_aggregation_relationship =
                    (hint == common::model::relationship_t::kAggregation);
            }
            else {
                if (template_argument.is_function_template())
                    hint = common::model::relationship_t::kDependency;

                added_aggregation_relationship =
                    template_argument.find_nested_relationships(
                        nested_relationships, hint, should_include);
            }
        }
    }

    return added_aggregation_relationship;
}

bool template_parameter::is_template_parameter() const
{
    return is_template_parameter_;
}

void template_parameter::is_template_parameter(bool is_template_parameter)
{
    is_template_parameter_ = is_template_parameter;
}

bool template_parameter::is_template_template_parameter() const
{
    return is_template_template_parameter_;
}

void template_parameter::is_template_template_parameter(
    bool is_template_template_parameter)
{
    is_template_template_parameter_ = is_template_template_parameter;
}

void template_parameter::set_concept_constraint(std::string constraint)
{
    concept_constraint_ = std::move(constraint);
}

const std::optional<std::string> &template_parameter::concept_constraint() const
{
    return concept_constraint_;
}

bool template_parameter::is_association() const
{
    return std::any_of(
        deduced_context().begin(), deduced_context().end(), [](const auto &c) {
            return c.pr == rpqualifier::kPointer ||
                c.pr == rpqualifier::kLValueReference;
        });
}

template_parameter_kind_t template_parameter::kind() const { return kind_; }

void template_parameter::set_kind(template_parameter_kind_t kind)
{
    kind_ = kind;
}

bool template_parameter::is_unexposed() const { return is_unexposed_; }

void template_parameter::set_unexposed(bool unexposed)
{
    is_unexposed_ = unexposed;
}

void template_parameter::is_function_template(bool ft)
{
    is_function_template_ = ft;
}
bool template_parameter::is_function_template() const
{
    return is_function_template_;
}

void template_parameter::is_member_pointer(bool m) { is_member_pointer_ = m; }
bool template_parameter::is_member_pointer() const
{
    return is_member_pointer_;
}

void template_parameter::is_data_pointer(bool m) { is_data_pointer_ = m; }
bool template_parameter::is_data_pointer() const { return is_data_pointer_; }

void template_parameter::is_array(bool a) { is_array_ = a; }
bool template_parameter::is_array() const { return is_array_; }

void template_parameter::push_context(const context &q)
{
    context_.push_front(q);
}

const std::deque<context> &template_parameter::deduced_context() const
{
    return context_;
}

void template_parameter::deduced_context(std::deque<context> c)
{
    context_ = std::move(c);
}

void template_parameter::is_ellipsis(bool e) { is_ellipsis_ = e; }

bool template_parameter::is_ellipsis() const { return is_ellipsis_; }

int calculate_template_params_specialization_match(
    const std::vector<template_parameter> &specialization_params,
    const std::vector<template_parameter> &template_params)
{
    int res{0};

    if (specialization_params.size() != template_params.size() &&
        !std::any_of(template_params.begin(), template_params.end(),
            [](const auto &t) { return t.is_variadic(); })) {
        return 0;
    }

    if (!specialization_params.empty() && !template_params.empty()) {
        auto template_index{0U};
        auto arg_index{0U};

        while (arg_index < specialization_params.size() &&
            template_index < template_params.size()) {
            auto match = specialization_params.at(arg_index)
                             .calculate_specialization_match(
                                 template_params.at(template_index));

            if (match == 0) {
                // If any of the matches is 0 - the entire match fails
                return 0;
            }

            // Add 1 point if the current specialization param is an argument
            // as it's a more specific match than 2 template params
            if (!specialization_params.at(arg_index).is_template_parameter())
                res++;

            // Add 1 point if the current template param is an argument
            // as it's a more specific match than 2 template params
            if (!template_params.at(template_index).is_template_parameter())
                res++;

            if (!template_params.at(template_index).is_variadic())
                template_index++;

            res += match;

            arg_index++;
        }

        if (arg_index == specialization_params.size()) {
            // Check also backwards to make sure that trailing non-variadic
            // params match after a variadic parameter
            template_index = template_params.size() - 1;
            arg_index = specialization_params.size() - 1;

            while (true) {
                auto match = specialization_params.at(arg_index)
                                 .calculate_specialization_match(
                                     template_params.at(template_index));
                if (match == 0) {
                    return 0;
                }

                if (arg_index == 0 || template_index == 0)
                    break;

                arg_index--;

                if (!template_params.at(template_index).is_variadic())
                    template_index--;
                else
                    break;
            }

            return res;
        }

        return 0;
    }

    return 0;
}

} // namespace clanguml::common::model
