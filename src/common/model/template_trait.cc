/**
 * @file src/common/model/template_trait.cc
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

void template_trait::add_template(template_parameter &&tmplt)
{
    templates_.push_back(std::move(tmplt));
}

const std::vector<template_parameter> &template_trait::template_params() const
{
    return templates_;
}

int template_trait::calculate_template_specialization_match(
    const template_trait &base_template) const
{
    return calculate_template_params_specialization_match(
        template_params(), base_template.template_params());
}

} // namespace clanguml::common::model