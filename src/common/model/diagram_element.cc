/**
 * @file src/common/model/diagram_element.cc
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

#include "diagram_element.h"

#include "common/model/filters/diagram_filter.h"
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

void diagram_element::append(const decorated_element &e)
{
    decorated_element::append(e);
}

bool diagram_element::is_nested() const { return nested_; }

void diagram_element::nested(bool nested) { nested_ = nested; }

bool diagram_element::complete() const { return complete_; }

void diagram_element::complete(bool completed) { complete_ = completed; }

void diagram_element::apply_filter(
    const diagram_filter &filter, const std::set<eid_t> &removed)
{
}

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
