/**
 * src/uml/class_diagram/model/diagram.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

std::string diagram::name() const { return name_; }

void diagram::set_name(const std::string &name) { name_ = name; }

const std::vector<class_> diagram::classes() const { return classes_; }

const std::vector<enum_> diagram::enums() const { return enums_; }

bool diagram::has_class(const class_ &c) const
{
    return std::any_of(classes_.cbegin(), classes_.cend(),
        [&c](const auto &cc) { return cc.full_name() == c.full_name(); });
}

void diagram::add_type_alias(type_alias &&ta)
{
    LOG_DBG("Adding global alias: {} -> {}", ta.alias(), ta.underlying_type());

    type_aliases_[ta.alias()] = std::move(ta);
}

void diagram::add_class(class_ &&c)
{
    LOG_DBG("Adding class: {}, {}", c.name(), c.full_name());
    if (!has_class(c))
        classes_.emplace_back(std::move(c));
    else
        LOG_DBG("Class {} ({}) already in the model", c.name(), c.full_name());
}

void diagram::add_enum(enum_ &&e)
{
    LOG_DBG("Adding enum: {}", e.name());
    auto it = std::find(enums_.begin(), enums_.end(), e);
    if (it == enums_.end())
        enums_.emplace_back(std::move(e));
    else
        LOG_DBG("Enum {} already in the model", e.name());
}

std::string diagram::to_alias(const std::string &full_name) const
{
    LOG_DBG("Looking for alias for {}", full_name);

    for (const auto &c : classes_) {
        if (c.full_name() == full_name) {
            return c.alias();
        }
    }

    for (const auto &e : enums_) {
        if (e.full_name() == full_name) {
            return e.alias();
        }
    }

    throw error::uml_alias_missing(
        fmt::format("Missing alias for {}", full_name));
}
}
