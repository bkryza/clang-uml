/**
 * @file src/class_diagram/model/concept.cc
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

#include "concept.h"
#include "method_parameter.h"

#include <sstream>

namespace clanguml::class_diagram::model {

concept_::concept_(const common::model::namespace_ &using_namespace)
    : element{using_namespace}
{
}

bool operator==(const concept_ &l, const concept_ &r)
{
    return l.id() == r.id();
}

std::string concept_::full_name_no_ns() const
{
    using namespace clanguml::util;

    std::ostringstream ostr;

    ostr << name();

    render_template_params(ostr, using_namespace(), false);

    return ostr.str();
}

std::string concept_::full_name(bool relative) const
{
    using namespace clanguml::util;
    using clanguml::common::model::namespace_;

    std::ostringstream ostr;

    ostr << name_and_ns();

    render_template_params(ostr, using_namespace(), relative);

    std::string res;

    if (relative)
        res = using_namespace().relative(ostr.str());
    else
        res = ostr.str();

    if (res.empty())
        return "<<anonymous>>";

    return res;
}

void concept_::add_parameter(const method_parameter &mp)
{
    requires_parameters_.emplace_back(mp);
}

const std::vector<method_parameter> &concept_::requires_parameters() const
{
    return requires_parameters_;
}

void concept_::add_statement(std::string stmt)
{
    requires_statements_.emplace_back(std::move(stmt));
}

const std::vector<std::string> &concept_::requires_statements() const
{
    return requires_statements_;
}

} // namespace clanguml::class_diagram::model
