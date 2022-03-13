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

void class_::add_template(class_template tmplt)
{
    templates_.emplace_back(std::move(tmplt));
}

const std::vector<class_member> &class_::members() const { return members_; }

const std::vector<class_method> &class_::methods() const { return methods_; }

const std::vector<class_parent> &class_::parents() const { return bases_; }

const std::vector<class_template> &class_::templates() const
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
    return l.full_name() == r.full_name();
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

    render_template_params(ostr);

    return ostr.str();
}

std::string class_::full_name(bool relative) const
{
    using namespace clanguml::util;
    using clanguml::common::model::namespace_;

    std::ostringstream ostr;

    ostr << name_and_ns();
    render_template_params(ostr);

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
    std::ostringstream &ostr) const
{
    using clanguml::common::model::namespace_;

    if (!templates_.empty()) {
        std::vector<std::string> tnames;
        std::transform(templates_.cbegin(), templates_.cend(),
            std::back_inserter(tnames), [this](const auto &tmplt) {
                return tmplt.to_string(using_namespace());
            });
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
}
