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

namespace clanguml::class_diagram::model {

const std::vector<type_safe::object_ref<class_>> diagram::classes() const
{
    return classes_;
}

const std::vector<type_safe::object_ref<enum_>> diagram::enums() const
{
    return enums_;
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

void diagram::add_type_alias(std::unique_ptr<type_alias> &&ta)
{
    LOG_DBG(
        "Adding global alias: {} -> {}", ta->alias(), ta->underlying_type());

    type_aliases_[ta->alias()] = std::move(ta);
}

void diagram::add_package(std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding namespace package: {}, {}", p->name(), p->full_name(true));

    add_element(p->get_namespace(), std::move(p));
}

void diagram::add_class(std::unique_ptr<class_> &&c)
{
    LOG_DBG("Adding class: {}, {}", c->name(), c->full_name());

    if (!has_class(*c)) {
        classes_.emplace_back(*c);
        add_element(c->get_namespace(), std::move(c));
    }
    else
        LOG_DBG(
            "Class {} ({}) already in the model", c->name(), c->full_name());
}

void diagram::add_enum(std::unique_ptr<enum_> &&e)
{
    LOG_DBG("Adding enum: {}", e->name());

    if (!has_enum(*e)) {
        enums_.emplace_back(*e);
        add_element(e->get_namespace(), std::move(e));
    }
    else
        LOG_DBG("Enum {} already in the model", e->name());
}

std::string diagram::to_alias(const std::string &full_name) const
{
    LOG_DBG("Looking for alias for {}", full_name);

    for (const auto &c : classes_) {
        if (c->full_name() == full_name) {
            return c->alias();
        }
    }

    for (const auto &e : enums_) {
        if (e->full_name() == full_name) {
            return e->alias();
        }
    }

    throw error::uml_alias_missing(
        fmt::format("Missing alias for {}", full_name));
}
}
