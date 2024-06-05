/**
 * @file src/class_diagram/model/diagram.cc
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

#include "common/model/diagram_filter.h"
#include "util/error.h"
#include "util/util.h"

namespace clanguml::class_diagram::model {

bool diagram::should_include(const class_member &m) const
{
    return filter().should_include(m);
}

bool diagram::should_include(const class_method &m) const
{
    return filter().should_include(m);
}

const common::reference_vector<class_> &diagram::classes() const
{
    return element_view<class_>::view();
}

const common::reference_vector<enum_> &diagram::enums() const
{
    return element_view<enum_>::view();
}

const common::reference_vector<concept_> &diagram::concepts() const
{
    return element_view<concept_>::view();
}

common::model::diagram_t diagram::type() const
{
    return common::model::diagram_t::kClass;
}

common::optional_ref<clanguml::common::model::diagram_element> diagram::get(
    const std::string &full_name) const
{
    common::optional_ref<clanguml::common::model::diagram_element> res =
        find<class_>(full_name);

    if (res.has_value())
        return res;

    res = find<enum_>(full_name);

    if (res.has_value())
        return res;

    res = find<concept_>(full_name);

    return res;
}

common::optional_ref<clanguml::common::model::diagram_element> diagram::get(
    const eid_t id) const
{
    common::optional_ref<clanguml::common::model::diagram_element> res;

    res = find<class_>(id);

    if (res.has_value())
        return res;

    res = find<enum_>(id);

    if (res.has_value())
        return res;

    res = find<concept_>(id);

    return res;
}

template <>
bool diagram::add_with_namespace_path<common::model::package>(
    std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding namespace package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();

    return add_element(ns, std::move(p));
}

template <>
bool diagram::add_with_filesystem_path<common::model::package>(
    const common::model::path & /*parent_path*/,
    std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding filesystem package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();

    return add_element(ns, std::move(p));
}

template <>
bool diagram::add_with_module_path<common::model::package>(
    const common::model::path & /*parent_path*/,
    std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding module package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();

    return add_element(ns, std::move(p));
}

void diagram::get_parents(
    clanguml::common::reference_set<class_> &parents) const
{
    bool found_new{false};
    for (const auto &parent : parents) {
        for (const auto &pp : parent.get().parents()) {
            auto p = find<class_>(pp.id());

            if (p.has_value()) {
                auto [it, found] = parents.emplace(std::ref(p.value()));
                if (found)
                    found_new = true;
            }
            else {
                LOG_WARN("Couldn't find class representing base class: {} [{}]",
                    pp.name(), pp.id());
            }
        }
    }

    if (found_new) {
        get_parents(parents);
    }
}

bool diagram::has_element(eid_t id) const
{
    const auto has_class = std::any_of(classes().begin(), classes().end(),
        [id](const auto &c) { return c.get().id() == id; });

    if (has_class)
        return true;

    const auto has_concept = std::any_of(classes().begin(), classes().end(),
        [id](const auto &c) { return c.get().id() == id; });

    if (has_concept)
        return true;

    return std::any_of(enums().begin(), enums().end(),
        [id](const auto &c) { return c.get().id() == id; });
}

std::string diagram::to_alias(eid_t id) const
{
    LOG_DBG("Looking for alias for {}", id);

    for (const auto &c : classes()) {
        if (c.get().id() == id) {
            return c.get().alias();
        }
    }

    for (const auto &e : enums()) {
        if (e.get().id() == id)
            return e.get().alias();
    }

    for (const auto &c : concepts()) {
        if (c.get().id() == id)
            return c.get().alias();
    }

    throw error::uml_alias_missing(fmt::format("Missing alias for {}", id));
}

inja::json diagram::context() const
{
    inja::json ctx;
    ctx["name"] = name();
    ctx["type"] = "class";

    inja::json::array_t elements{};

    // Add classes
    for (const auto &c : classes()) {
        elements.emplace_back(c.get().context());
    }

    // Add enums
    for (const auto &e : enums()) {
        elements.emplace_back(e.get().context());
    }

    // Add enums
    for (const auto &c : concepts()) {
        elements.emplace_back(c.get().context());
    }

    ctx["elements"] = elements;

    return ctx;
}

void diagram::remove_redundant_dependencies()
{
    using common::eid_t;
    using common::model::relationship;
    using common::model::relationship_t;

    for (auto &c : element_view<class_>::view()) {
        std::set<eid_t> dependency_relationships_to_remove;

        for (auto &r : c.get().relationships()) {
            if (r.type() != relationship_t::kDependency)
                dependency_relationships_to_remove.emplace(r.destination());
        }

        for (const auto &base : c.get().parents()) {
            dependency_relationships_to_remove.emplace(base.id());
        }

        util::erase_if(c.get().relationships(),
            [&dependency_relationships_to_remove](const auto &r) {
                return r.type() == relationship_t::kDependency &&
                    dependency_relationships_to_remove.count(r.destination()) >
                    0;
            });
    }
}

bool diagram::is_empty() const
{
    return element_view<class_>::is_empty() &&
        element_view<enum_>::is_empty() && element_view<concept_>::is_empty();
}
} // namespace clanguml::class_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::class_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kClass;
}
}
