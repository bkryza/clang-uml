/**
 * src/class_diagram/model/diagram.cc
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

#include <cassert>

namespace clanguml::class_diagram::model {

const std::vector<std::reference_wrapper<const class_>> &
diagram::classes() const
{
    return classes_;
}

const std::vector<std::reference_wrapper<const enum_>> &diagram::enums() const
{
    return enums_;
}

common::model::diagram_t diagram::type() const
{
    return common::model::diagram_t::kClass;
}

std::optional<
    std::reference_wrapper<const clanguml::common::model::diagram_element>>
diagram::get(const std::string &full_name) const
{
    // type_safe::optional_ref<const clanguml::common::model::diagram_element>
    // res;
    std::optional<
        std::reference_wrapper<const clanguml::common::model::diagram_element>>
        res;

    res = get_class(full_name);

    if (res.has_value())
        return res;

    res = get_enum(full_name);

    return res;
}

bool diagram::has_class(const class_ &c) const
{
    return std::any_of(classes_.cbegin(), classes_.cend(),
        [&c](const auto &cc) { return cc.get() == c; });
}

bool diagram::has_enum(const enum_ &e) const
{
    return std::any_of(enums_.cbegin(), enums_.cend(),
        [&e](const auto &ee) { return ee.get().full_name() == e.full_name(); });
}

std::optional<std::reference_wrapper<const class_>> diagram::get_class(
    const std::string &name) const
{
    for (const auto &c : classes_) {
        const auto full_name = c.get().full_name(false);
        if (name ==
            "clanguml::t00012::C<std::map<int,std::vector<std::vector<std::"
            "vector<std::string>>>>,3,3,3>")
            LOG_ERROR("Comparing {} with {}", full_name, name);

        if (full_name == name) {
            return {c};
        }
    }

    return {};
}

std::optional<std::reference_wrapper<const class_>> diagram::get_class(
    clanguml::common::model::diagram_element::id_t id) const
{
    for (const auto &c : classes_) {
        if (c.get().id() == id) {
            return {c};
        }
    }

    return {};
}

std::optional<std::reference_wrapper<const enum_>> diagram::get_enum(
    const std::string &name) const
{
    for (const auto &e : enums_) {
        if (e.get().full_name(false) == name) {
            return {e};
        }
    }

    return {};
}

void diagram::add_type_alias(std::unique_ptr<type_alias> &&ta)
{
    LOG_DBG(
        "Adding global alias: {} -> {}", ta->alias(), ta->underlying_type());

    type_aliases_[ta->alias()] = std::move(ta);
}

bool diagram::add_package(std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding namespace package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();

    return add_element(ns, std::move(p));
}

bool diagram::add_class(std::unique_ptr<class_> &&c)
{
    const auto base_name = c->name();
    const auto full_name = c->full_name(false);

    LOG_DBG("Adding class: {}::{}, {}", c->get_namespace().to_string(),
        base_name, full_name);

    if (util::contains(base_name, "::"))
        throw std::runtime_error("Name cannot contain namespace: " + base_name);

    if (util::contains(base_name, "<"))
        throw std::runtime_error("Name cannot contain <: " + base_name);

    if (util::contains(base_name, "*"))
        throw std::runtime_error("Name cannot contain *: " + base_name);

    const auto ns = c->get_relative_namespace();
    auto name = base_name;
    auto name_with_ns = c->name_and_ns();
    auto name_and_ns = ns | name;
    const auto &cc = *c;

    auto cc_ref = std::ref(cc);

    if (!has_class(cc)) {
        if (add_element(ns, std::move(c)))
            classes_.push_back(std::move(cc_ref));

        const auto &el = get_element<class_>(name_and_ns).value();

        if ((el.name() != name) || !(el.get_relative_namespace() == ns))
            throw std::runtime_error(
                "Invalid element stored in the diagram tree");

        return true;
    }

    LOG_DBG("Class {} ({}) already in the model", base_name, full_name);

    return false;
}

bool diagram::add_enum(std::unique_ptr<enum_> &&e)
{
    const auto full_name = e->name();

    LOG_DBG("Adding enum: {}", full_name);

    assert(!util::contains(e->name(), "::"));

    auto e_ref = std::ref(*e);
    auto ns = e->get_relative_namespace();

    if (!has_enum(*e)) {
        if (add_element(ns, std::move(e))) {
            enums_.emplace_back(std::move(e_ref));
            return true;
        }
    }

    LOG_DBG("Enum {} already in the model", full_name);

    return false;
}

void diagram::get_parents(
    clanguml::common::reference_set<class_> &parents) const
{
    bool found_new{false};
    for (const auto &parent : parents) {
        for (const auto &pp : parent.get().parents()) {
            const auto p = get_class(pp.name());
            if (p.has_value()) {
                auto [it, found] = parents.emplace(p.value());
                if (found)
                    found_new = true;
            }
        }
    }

    if (found_new) {
        get_parents(parents);
    }
}

bool diagram::has_element(
    clanguml::common::model::diagram_element::id_t id) const
{
    for (const auto &c : classes_) {
        if (c.get().id() == id)
            return true;
    }

    for (const auto &c : enums_) {
        if (c.get().id() == id)
            return true;
    }

    return false;
}

std::string diagram::to_alias(
    clanguml::common::model::diagram_element::id_t id) const
{
    LOG_DBG("Looking for alias for {}", id);

    for (const auto &c : classes_) {
        if (c.get().id() == id) {
            return c.get().alias();
        }
    }

    for (const auto &e : enums_) {
        if (e.get().id() == id)
            return e.get().alias();
    }

    throw error::uml_alias_missing(fmt::format("Missing alias for {}", id));
}

}

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::class_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kClass;
}
}
