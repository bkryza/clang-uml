/**
 * src/common/model/template_trait.cc
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

#include "common/model/template_trait.h"

namespace clanguml::common::model {

std::ostream &template_trait::render_template_params(std::ostream &ostr,
    const common::model::namespace_ &using_namespace, bool relative) const
{
    using clanguml::common::model::namespace_;

    if (!templates_.empty()) {
        std::vector<std::string> tnames;

        std::transform(templates_.cbegin(), templates_.cend(),
            std::back_inserter(tnames),
            [ns = using_namespace, relative](
                const auto &tmplt) { return tmplt.to_string(ns, relative); });

        ostr << fmt::format("<{}>", fmt::join(tnames, ","));
    }

    return ostr;
}

void template_trait::set_base_template(const std::string &full_name)
{
    base_template_full_name_ = full_name;
}

std::string template_trait::base_template() const
{
    return base_template_full_name_;
}

void template_trait::add_template(template_parameter &&tmplt)
{
    templates_.push_back(std::move(tmplt));
}

bool template_trait::is_implicit() const { return is_implicit_; }

void template_trait::set_implicit(bool implicit) { is_implicit_ = implicit; }

const std::vector<template_parameter> &template_trait::templates() const
{
    return templates_;
}

int template_trait::calculate_template_specialization_match(
    const template_trait &other) const
{
    int res{0};

    // Iterate over all template arguments
    for (auto i = 0U; i < other.templates().size(); i++) {
        const auto &template_arg = templates().at(i);
        const auto &other_template_arg = other.templates().at(i);

        if (template_arg == other_template_arg) {
            res++;

            if (!template_arg.is_template_parameter())
                res++;

            if (!other_template_arg.is_template_parameter())
                res++;
        }
        else if (auto match = other_template_arg.calculate_specialization_match(
                     template_arg);
                 match > 0) {
            res += match;
        }
        else {
            res = 0;
            break;
        }
    }

    return res;
}

} // namespace clanguml::common::model