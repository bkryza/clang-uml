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
#include <iostream>

namespace clanguml::class_diagram::model {

const std::vector<type_safe::object_ref<const class_>> diagram::classes() const
{
    return classes_;
}

const std::vector<type_safe::object_ref<const enum_>> diagram::enums() const
{
    return enums_;
}

common::model::diagram_t diagram::type() const
{
    return common::model::diagram_t::kClass;
}

bool diagram::has_class(const class_ &c) const
{
    return std::any_of(classes_.cbegin(), classes_.cend(),
        [&c](const auto &cc) { return cc.get().full_name() == c.full_name(); });
}

bool diagram::has_enum(const enum_ &e) const
{
    return std::any_of(enums_.cbegin(), enums_.cend(),
        [&e](const auto &ee) { return ee.get().full_name() == e.full_name(); });
}

type_safe::optional_ref<const class_> diagram::get_class(
    const std::string &name) const
{
    for (const auto &c : classes_) {
        if (c.get().full_name(false) == name) {
            return {c};
        }
    }

    return type_safe::nullopt;
}

void diagram::add_type_alias(std::unique_ptr<type_alias> &&ta)
{
    LOG_DBG(
        "Adding global alias: {} -> {}", ta->alias(), ta->underlying_type());

    type_aliases_[ta->alias()] = std::move(ta);
}

void diagram::add_package(std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding namespace package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();
    add_element(ns, std::move(p));
}

void diagram::add_class(std::unique_ptr<class_> &&c)
{
    LOG_DBG("Adding class: {}::{}, {}", c->get_namespace().to_string(),
        c->name(), c->full_name());

    if (util::contains(c->name(), "::"))
        throw std::runtime_error("Name cannot contain namespace: " + c->name());

    if (util::contains(c->name(), "<"))
        throw std::runtime_error("Name cannot contain <: " + c->name());

    if (util::contains(c->name(), "*"))
        throw std::runtime_error("Name cannot contain *: " + c->name());

    if (!has_class(*c)) {
        classes_.emplace_back(*c);
        auto ns = c->get_relative_namespace();
        auto name = c->name();
        add_element(ns, std::move(c));
        ns |= name;
        const auto ccc = get_element<class_>(ns);
        assert(ccc.value().name() == name);
    }
    else
        LOG_DBG(
            "Class {} ({}) already in the model", c->name(), c->full_name());
}

void diagram::add_enum(std::unique_ptr<enum_> &&e)
{
    LOG_DBG("Adding enum: {}", e->name());

    assert(!util::contains(e->name(), "::"));

    if (!has_enum(*e)) {
        enums_.emplace_back(*e);
        auto ns = e->get_relative_namespace();
        add_element(ns, std::move(e));
    }
    else
        LOG_DBG("Enum {} already in the model", e->name());
}

void diagram::get_parents(
    std::unordered_set<type_safe::object_ref<const class_>> &parents) const
{
    bool found_new = false;
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

std::string diagram::to_alias(const std::string &full_name) const
{
    LOG_DBG("Looking for alias for {}", full_name);

    for (const auto &c : classes_) {
        const auto &cc = c.get();
        if (cc.full_name() == full_name) {
            return c->alias();
        }
    }

    for (const auto &e : enums_) {
        if (e.get().full_name() == full_name) {
            return e->alias();
        }
    }

    throw error::uml_alias_missing(
        fmt::format("Missing alias for {}", full_name));
}

}
