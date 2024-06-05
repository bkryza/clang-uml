/**
 * @file src/common/model/diagram_element.cc
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

#include "diagram_element.h"

#include "util/util.h"

#include <ostream>

namespace clanguml::common::model {

diagram_element::diagram_element() = default;

const eid_t &diagram_element::id() const { return id_; }

void diagram_element::set_id(eid_t id) { id_ = id; }

std::optional<eid_t> diagram_element::parent_element_id() const
{
    return parent_element_id_;
}

void diagram_element::set_parent_element_id(eid_t id)
{
    parent_element_id_ = id;
}

std::string diagram_element::alias() const
{
    // Only generate alias for global id's
    assert(id_.is_global());

    return fmt::format("C_{:022}", id_.value());
}

void diagram_element::add_relationship(relationship &&cr)
{
    if ((cr.type() == relationship_t::kInstantiation) &&
        (cr.destination() == id())) {
        LOG_DBG("Skipping self instantiation relationship for {}",
            cr.destination());
        return;
    }

    if (!util::contains(relationships_, cr)) {
        LOG_DBG("Adding relationship from: '{}' ({}) - {} - '{}'", id(),
            full_name(true), to_string(cr.type()), cr.destination());

        relationships_.emplace_back(std::move(cr));
    }
}

std::vector<relationship> &diagram_element::relationships()
{
    return relationships_;
}

const std::vector<relationship> &diagram_element::relationships() const
{
    return relationships_;
}

void diagram_element::append(const decorated_element &e)
{
    decorated_element::append(e);
}

inja::json diagram_element::context() const
{
    inja::json ctx;
    ctx["name"] = name();
    ctx["type"] = type_name();
    ctx["alias"] = alias();
    ctx["full_name"] = full_name(false);
    auto maybe_doxygen_link = doxygen_link();
    if (maybe_doxygen_link)
        ctx["doxygen_link"] = maybe_doxygen_link.value();

    return ctx;
}

bool diagram_element::is_nested() const { return nested_; }

void diagram_element::nested(bool nested) { nested_ = nested; }

bool diagram_element::complete() const { return complete_; }

void diagram_element::complete(bool completed) { complete_ = completed; }

bool operator==(const diagram_element &l, const diagram_element &r)
{
    return l.id() == r.id();
}

std::ostream &operator<<(std::ostream &out, const diagram_element &rhs)
{
    out << "(" << rhs.name() << ", full_name=[" << rhs.full_name(false) << "])";

    return out;
}

} // namespace clanguml::common::model
