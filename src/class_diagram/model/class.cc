/**
 * src/class_diagram/model/class.cc
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

#include "class.h"

#include "util/util.h"

#include <sstream>

namespace clanguml::class_diagram::model {

class_::class_(const common::model::namespace_ &using_namespace)
    : element{using_namespace}
{
}

bool class_::is_struct() const { return is_struct_; }

void class_::is_struct(bool is_struct) { is_struct_ = is_struct; }

bool class_::is_template() const { return is_template_; }

void class_::is_template(bool is_template) { is_template_ = is_template; }

bool class_::is_template_instantiation() const
{
    return is_template_instantiation_;
}

void class_::is_template_instantiation(bool is_template_instantiation)
{
    is_template_instantiation_ = is_template_instantiation;
}

void class_::add_member(class_member &&member)
{
    members_.emplace_back(std::move(member));
}

void class_::add_method(class_method &&method)
{
    methods_.emplace_back(std::move(method));
}

void class_::add_parent(class_parent &&parent)
{
    bases_.emplace_back(std::move(parent));
}

void class_::add_template(template_parameter tmplt)
{
    templates_.emplace_back(std::move(tmplt));
}

const std::vector<class_member> &class_::members() const { return members_; }

const std::vector<class_method> &class_::methods() const { return methods_; }

const std::vector<class_parent> &class_::parents() const { return bases_; }

const std::vector<template_parameter> &class_::templates() const
{
    return templates_;
}

void class_::set_base_template(const std::string &full_name)
{
    base_template_full_name_ = full_name;
}

std::string class_::base_template() const { return base_template_full_name_; }

bool operator==(const class_ &l, const class_ &r)
{
    return (l.name_and_ns() == r.name_and_ns()) &&
        (l.templates_ == r.templates_);
}

void class_::add_type_alias(type_alias &&ta)
{
    LOG_DBG("Adding class alias: {} -> {}", ta.alias(), ta.underlying_type());
    type_aliases_[ta.alias()] = std::move(ta);
}

std::string class_::full_name_no_ns() const
{
    using namespace clanguml::util;

    std::ostringstream ostr;

    ostr << name();

    render_template_params(ostr, false);

    return ostr.str();
}

std::string class_::full_name(bool relative) const
{
    using namespace clanguml::util;
    using clanguml::common::model::namespace_;

    std::ostringstream ostr;

    ostr << name_and_ns();
    render_template_params(ostr, relative);

    std::string res;

    if (relative)
        res = using_namespace().relative(ostr.str());
    else
        res = ostr.str();

    if (res.empty())
        return "<<anonymous>>";

    return res;
}

std::ostringstream &class_::render_template_params(
    std::ostringstream &ostr, bool relative) const
{
    using clanguml::common::model::namespace_;

    if (!templates_.empty()) {
        std::vector<std::string> tnames;
        std::vector<std::string> tnames_simplified;

        std::transform(templates_.cbegin(), templates_.cend(),
            std::back_inserter(tnames),
            [ns = using_namespace(), relative](
                const auto &tmplt) { return tmplt.to_string(ns, relative); });

        ostr << fmt::format("<{}>", fmt::join(tnames, ","));
    }

    return ostr;
}

bool class_::is_abstract() const
{
    // TODO check if all base abstract methods are overriden
    // with non-abstract methods
    return std::any_of(methods_.begin(), methods_.end(),
        [](const auto &method) { return method.is_pure_virtual(); });
}

int class_::calculate_template_specialization_match(
    const class_ &other, const std::string &full_name) const
{
    int res{};

    std::string left = name_and_ns();
    // TODO: handle variadic templates
    if ((name_and_ns() != full_name) ||
        (templates().size() != other.templates().size())) {
        return res;
    }

    // Iterate over all template arguments
    for (auto i = 0U; i < other.templates().size(); i++) {
        const auto &template_arg = templates().at(i);
        const auto &other_template_arg = other.templates().at(i);

        if (template_arg == other_template_arg) {
            res++;
        }
        else if (other_template_arg.is_specialization_of(template_arg)) {
            continue;
        }
        else {
            res = 0;
            break;
        }
    }

    return res;
}
}
