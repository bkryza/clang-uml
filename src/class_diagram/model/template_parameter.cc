/**
 * src/class_diagram/model/template_parameter.cc
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

#include "template_parameter.h"
#include <common/model/namespace.h>
#include <fmt/format.h>

namespace clanguml::class_diagram::model {

template_parameter::template_parameter(const std::string &type,
    const std::string &name, const std::string &default_value, bool is_variadic)
    : default_value_{default_value}
    , is_variadic_{is_variadic}
{
    set_name(name);
    set_type(type);
}

void template_parameter::set_type(const std::string &type) { type_ = type; }

std::string template_parameter::type() const { return type_; }

void template_parameter::set_name(const std::string &name)
{
    name_ = name;
    // TODO: Add a configurable mapping for simplifying non-interesting
    //       std templates
    util::replace_all(name_, "std::basic_string<char>", "std::string");
    util::replace_all(name_, "std::basic_string<wchar_t>", "std::wstring");
}

std::string template_parameter::name() const
{
    if (is_variadic_)
        return name_ + "...";

    return name_;
}

void template_parameter::set_default_value(const std::string &value)
{
    default_value_ = value;
}

std::string template_parameter::default_value() const { return default_value_; }

void template_parameter::is_variadic(bool is_variadic) noexcept
{
    is_variadic_ = is_variadic;
}

bool template_parameter::is_variadic() const noexcept { return is_variadic_; }

bool template_parameter::is_specialization_of(
    const template_parameter &ct) const
{
    if ((ct.is_template_parameter() || ct.is_template_template_parameter()) &&
        !is_template_parameter())
        return true;

    return false;
}

void template_parameter::add_template_param(template_parameter &&ct)
{
    template_params_.emplace_back(std::move(ct));
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
    const clanguml::common::model::namespace_ &using_namespace) const
{
    using clanguml::common::model::namespace_;

    std::string res;
    if (!type().empty())
        res += namespace_{type()}.relative_to(using_namespace).to_string();

    // Render nested template params
    if (!template_params_.empty()) {
        std::vector<std::string> params;
        for (const auto &template_param : template_params_) {
            params.push_back(template_param.to_string(using_namespace));
        }

        res += fmt::format("<{}>", fmt::join(params, ","));
    }

    if (!name().empty()) {
        if (!type().empty())
            res += " ";
        res += namespace_{name()}.relative_to(using_namespace).to_string();
    }

    if (!default_value().empty()) {
        res += "=";
        res += default_value();
    }

    return res;
}

}
