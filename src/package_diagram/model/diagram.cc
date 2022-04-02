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

void diagram::add_package(std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();

    packages_.emplace_back(*p);

    add_element(ns, std::move(p));
}

type_safe::optional_ref<const common::model::package> diagram::get_package(
    const std::string &name) const
{
    for (const auto &p : packages_) {
        if (p.get().full_name(false) == name) {
            return {p};
        }
    }

    return type_safe::nullopt;
}

type_safe::optional_ref<const clanguml::common::model::element> diagram::get(
    const std::string &full_name) const
{
    return get_package(full_name);
}

std::string diagram::to_alias(const std::string &full_name) const
{
    LOG_DBG("Looking for alias for {}", full_name);

    auto path = common::model::namespace_{full_name};

    if (path.is_empty())
        throw error::uml_alias_missing(
            fmt::format("Missing alias for '{}'", path.to_string()));

    auto package = get_element<common::model::package>(path);

    if (!package)
        throw error::uml_alias_missing(
            fmt::format("Missing alias for '{}'", path.to_string()));

    return package.value().alias();
}
}
