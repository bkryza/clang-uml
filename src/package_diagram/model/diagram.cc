/**
 * src/package_diagram/model/diagram.cc
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

#include "diagram.h"

#include "util/error.h"
#include "util/util.h"

namespace clanguml::package_diagram::model {

common::model::diagram_t diagram::type() const
{
    return common::model::diagram_t::kPackage;
}

const common::reference_vector<clanguml::common::model::package> &
diagram::packages() const
{
    return packages_;
}

void diagram::add_package(std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();

    packages_.emplace_back(*p);

    add_element(ns, std::move(p));
}

common::optional_ref<common::model::package> diagram::get_package(
    const std::string &name) const
{
    for (const auto &p : packages_) {
        auto p_full_name = p.get().full_name(false);
        if (p_full_name == name) {
            return {p};
        }
    }

    return {};
}

common::optional_ref<common::model::package> diagram::get_package(
    const clanguml::common::model::diagram_element::id_t id) const
{
    for (const auto &p : packages_) {
        if (p.get().id() == id) {
            return {p};
        }
    }

    return {};
}

common::optional_ref<clanguml::common::model::diagram_element> diagram::get(
    const std::string &full_name) const
{
    return get_package(full_name);
}

common::optional_ref<clanguml::common::model::diagram_element> diagram::get(
    const clanguml::common::model::diagram_element::id_t id) const
{
    return get_package(id);
}

std::string diagram::to_alias(
    const clanguml::common::model::diagram_element::id_t id) const
{
    LOG_DBG("Looking for alias for {}", id);

    for (const auto &p : packages_) {
        if (p.get().id() == id)
            return p.get().alias();
    }

    return {};
}
}

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::package_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kPackage;
}
}