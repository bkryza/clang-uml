/**
 * @file src/class_diagram/model/class.cc
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

#include "class.h"

#include "util/util.h"

#include <sstream>

namespace clanguml::class_diagram::model {

class_::class_(const common::model::namespace_ &using_namespace)
    : template_element{using_namespace}
{
}

bool class_::is_struct() const { return is_struct_; }

void class_::is_struct(bool is_struct) { is_struct_ = is_struct; }

bool class_::is_union() const { return is_union_; }

void class_::is_union(bool is_union) { is_union_ = is_union; }

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
    for (const auto &p : bases_) {
        if (p.id() == parent.id()) {
            return;
        }
    }

    bases_.emplace_back(std::move(parent));
}

const std::vector<class_member> &class_::members() const { return members_; }

const std::vector<class_method> &class_::methods() const { return methods_; }

const std::vector<class_parent> &class_::parents() const { return bases_; }

bool operator==(const class_ &l, const class_ &r) { return l.id() == r.id(); }

std::string class_::full_name_no_ns() const
{
    using namespace clanguml::util;

    std::ostringstream ostr;

    ostr << name();

    render_template_params(ostr, using_namespace(), true);

    return ostr.str();
}

std::string class_::full_name(bool relative) const
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

bool class_::is_abstract() const
{
    // TODO check if all base abstract methods are overriden
    // with non-abstract methods
    return std::any_of(methods_.begin(), methods_.end(),
        [](const auto &method) { return method.is_pure_virtual(); });
}

std::optional<std::string> class_::doxygen_link() const
{
    const auto *type = is_struct() ? "struct" : "class";

    auto name = name_and_ns();
    util::replace_all(name, "_", "__");
    util::replace_all(name, "::", "_1_1");
    util::replace_all(name, "##", "_1_1"); // nested classes
    return fmt::format("{}{}.html", type, name);
}
} // namespace clanguml::class_diagram::model
