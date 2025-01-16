/**
 * @file src/package_diagram/model/diagram.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include "common/model/filters/diagram_filter.h"
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

void diagram::apply_filter()
{
    // Remove elements by traversing down into packages until no elements
    // were removed. This is necessary to remove all empty packages.
    while (true) {
        auto previous_to_remove_size = packages().size();

        // First find all element ids which should be removed
        std::set<eid_t> to_remove;

        for (const auto &c : packages()) {
            if (!filter().should_include(c.get())) {
                to_remove.emplace(c.get().id());
            }
        }

        element_view<package>::remove(to_remove);

        nested_trait_ns::remove(to_remove);

        for (const auto &c : packages()) {
            c.get().apply_filter(filter(), to_remove);
        }

        // If this loop didn't remove anything - stop it
        if (previous_to_remove_size == packages().size())
            break;
    }
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