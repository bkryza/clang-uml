/**
 * @file src/class_diagram/model/diagram.cc
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

namespace clanguml::class_diagram::model {

bool diagram::should_include(const class_member &m) const
{
    return filter().should_include(m);
}

bool diagram::should_include(const class_method &m) const
{
    return filter().should_include(m);
}

bool diagram::should_include(const objc_member &m) const
{
    return filter().should_include(m);
}

bool diagram::should_include(const objc_method &m) const
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

const common::reference_vector<objc_interface> &diagram::objc_interfaces() const
{
    return element_view<objc_interface>::view();
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

    if (res.has_value())
        return res;

    res = find<objc_interface>(full_name);

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

    if (res.has_value())
        return res;

    res = find<objc_interface>(id);

    return res;
}

void diagram::add_class(std::unique_ptr<class_> &&c)
{
    assert(c->id().value() != 0);

    if ((config().generate_packages() &&
            config().package_type() == config::package_type_t::kDirectory)) {
        assert(!c->file().empty());

        const auto file = config().make_path_relative(c->file());

        common::model::path p{
            file.string(), common::model::path_type::kFilesystem};
        p.pop_back();

        add(p, std::move(c));
    }
    else if ((config().generate_packages() &&
                 config().package_type() == config::package_type_t::kModule)) {

        const auto module_path = config().make_module_relative(c->module());

        common::model::path p{module_path, common::model::path_type::kModule};

        add(p, std::move(c));
    }
    else {
        add(c->path(), std::move(c));
    }
}

void diagram::add_objc_interface(std::unique_ptr<objc_interface> &&c)
{
    if ((config().generate_packages() &&
            config().package_type() == config::package_type_t::kDirectory)) {
        assert(!c->file().empty());

        const auto file = config().make_path_relative(c->file());

        common::model::path p{
            file.string(), common::model::path_type::kFilesystem};
        p.pop_back();

        add(p, std::move(c));
    }
    else {
        add(c->path(), std::move(c));
    }
}

void diagram::add_enum(std::unique_ptr<enum_> &&e)
{
    if ((config().generate_packages() &&
            config().package_type() == config::package_type_t::kDirectory)) {
        assert(!e->file().empty());

        const auto file = config().make_path_relative(e->file());

        common::model::path p{
            file.string(), common::model::path_type::kFilesystem};
        p.pop_back();

        add(p, std::move(e));
    }
    else if ((config().generate_packages() &&
                 config().package_type() == config::package_type_t::kModule)) {

        const auto module_path = config().make_module_relative(e->module());

        common::model::path p{module_path, common::model::path_type::kModule};

        add(p, std::move(e));
    }
    else {
        add(e->path(), std::move(e));
    }
}

void diagram::add_concept(std::unique_ptr<concept_> &&c)
{
    if ((config().generate_packages() &&
            config().package_type() == config::package_type_t::kDirectory)) {
        assert(!c->file().empty());

        const auto file = config().make_path_relative(c->file());

        common::model::path p{
            file.string(), common::model::path_type::kFilesystem};
        p.pop_back();

        add(p, std::move(c));
    }
    else if ((config().generate_packages() &&
                 config().package_type() == config::package_type_t::kModule)) {

        const auto module_path = config().make_module_relative(c->module());

        common::model::path p{module_path, common::model::path_type::kModule};

        add(p, std::move(c));
    }
    else {
        add(c->path(), std::move(c));
    }
}

template <>
bool diagram::add_with_namespace_path<common::model::package>(
    std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding namespace package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();
    if (common::model::needs_root_prefix(*p) && p->get_namespace().is_empty())
        p->is_root(true);

    return add_element(ns, std::move(p));
}

template <>
bool diagram::add_with_filesystem_path<common::model::package>(
    const common::model::path & /*parent_path*/,
    std::unique_ptr<common::model::package> &&p)
{
    auto ns = p->get_relative_namespace();

    LOG_DBG("Adding filesystem package: {}, {}, {}", p->name(),
        p->full_name(true), ns.to_string());

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
        for (const auto &rel : relationships(parent.get().id())) {
            if (rel.type() != common::model::relationship_t::kExtension)
                continue;

            auto p = find<class_>(rel.destination());

            if (p.has_value()) {
                auto [it, found] = parents.emplace(std::ref(p.value()));
                if (found)
                    found_new = true;
            }
            else {
                LOG_WARN("Couldn't find class representing base class: {}",
                    rel.destination().value());
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

    const auto has_enum = std::any_of(enums().begin(), enums().end(),
        [id](const auto &c) { return c.get().id() == id; });

    if (has_enum)
        return true;

    return std::any_of(objc_interfaces().begin(), objc_interfaces().end(),
        [id](const auto &c) { return c.get().id() == id; });
}

void diagram::remove_redundant_dependencies()
{
    using common::eid_t;
    using common::model::relationship;
    using common::model::relationship_t;

    for_all_elements([&](auto &&elements_view) mutable {
        for (const auto &el : elements_view) {
            std::set<eid_t> dependency_relationships_to_remove;

            for (auto &r : relationships(el.get().id())) {
                if (r.type() != relationship_t::kDependency)
                    dependency_relationships_to_remove.emplace(r.destination());
            }

            util::erase_if(relationships(),
                [&dependency_relationships_to_remove, &el](const auto &r) {
                    if (r.type() != relationship_t::kDependency)
                        return false;

                    if (r.source() != el.get().id())
                        return false;

                    auto has_another_relationship_to_destination =
                        dependency_relationships_to_remove.count(
                            r.destination()) > 0;
                    auto is_self_dependency = r.destination() == el.get().id();

                    return has_another_relationship_to_destination ||
                        is_self_dependency;
                });
        }
    });
}

bool diagram::is_empty() const
{
    return element_view<class_>::is_empty() &&
        element_view<enum_>::is_empty() && element_view<concept_>::is_empty() &&
        element_view<objc_interface>::is_empty();
}

void diagram::append(diagram &&other)
{
    clanguml::common::model::diagram::append(
        dynamic_cast<clanguml::common::model::diagram &&>(other));

    element_views<class_, enum_, concept_, objc_interface>::append(
        dynamic_cast<element_views<class_, enum_, concept_, objc_interface> &&>(
            other));

    for (const auto &ae : other.added_elements_) {
        added_elements_.emplace(ae);
    }

    nested_trait_t::append(dynamic_cast<nested_trait_t &&>(std::move(other)));
}
} // namespace clanguml::class_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::class_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kClass;
}
}
