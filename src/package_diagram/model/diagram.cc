/**
 * @file src/package_diagram/model/diagram.cc
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
    return element_view<package>::view();
}

common::optional_ref<clanguml::common::model::diagram_element> diagram::get(
    const std::string &full_name) const
{
    return find<package>(full_name);
}

common::optional_ref<clanguml::common::model::diagram_element> diagram::get(
    const eid_t id) const
{
    return find<package>(id);
}

std::string diagram::to_alias(const eid_t id) const
{
    LOG_DBG("Looking for alias for {}", id);

    auto p = find<package>(id);
    if (p.has_value() && p.value().id() == id)
        return p.value().alias();

    return {};
}

inja::json diagram::context() const
{
    inja::json ctx;
    ctx["name"] = name();
    ctx["type"] = "package";

    inja::json::array_t elements{};

    for (const auto &p : packages()) {
        elements.emplace_back(p.get().context());
    }

    ctx["elements"] = elements;

    return ctx;
}

bool diagram::is_empty() const { return element_view<package>::is_empty(); }
} // namespace clanguml::package_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::package_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kPackage;
}
}