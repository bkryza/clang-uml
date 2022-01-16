/**
 * src/common/model/class_relationship.cc
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

#include "relationship.h"

namespace clanguml::common::model {

std::string to_string(relationship_t r)
{
    switch (r) {
    case relationship_t::kNone:
        return "none";
    case relationship_t::kExtension:
        return "extension";
    case relationship_t::kComposition:
        return "composition";
    case relationship_t::kAggregation:
        return "aggregation";
    case relationship_t::kContainment:
        return "containment";
    case relationship_t::kOwnership:
        return "ownership";
    case relationship_t::kAssociation:
        return "association";
    case relationship_t::kInstantiation:
        return "instantiation";
    case relationship_t::kFriendship:
        return "frendship";
    case relationship_t::kDependency:
        return "dependency";
    default:
        return "invalid";
    }
}

relationship::relationship(relationship_t type,
    const std::string &destination, scope_t scope, const std::string &label,
    const std::string &multiplicity_source,
    const std::string &multiplicity_destination)
    : type_{type}
    , destination_{destination}
    , scope_{scope}
    , label_{label}
    , multiplicity_source_{multiplicity_source}
    , multiplicity_destination_{multiplicity_destination}
{
}

void relationship::set_type(relationship_t type) noexcept
{
    type_ = type;
}

relationship_t relationship::type() const noexcept { return type_; }

void relationship::set_destination(const std::string &destination)
{
    destination_ = destination;
}

std::string relationship::destination() const { return destination_; }

void relationship::set_multiplicity_source(
    const std::string &multiplicity_source)
{
    multiplicity_source_ = multiplicity_source;
}

std::string relationship::multiplicity_source() const
{
    return multiplicity_source_;
}

void relationship::set_multiplicity_destination(
    const std::string &multiplicity_destination)
{
    multiplicity_destination_ = multiplicity_destination;
}

std::string relationship::multiplicity_destination() const
{
    return multiplicity_destination_;
}

void relationship::set_label(const std::string &label) { label_ = label; }

std::string relationship::label() const { return label_; }

void relationship::set_scope(scope_t scope) noexcept { scope_ = scope; }

scope_t relationship::scope() const noexcept { return scope_; }

bool operator==(const relationship &l, const relationship &r)
{
    return l.type() == r.type() && l.destination() == r.destination() &&
        l.label() == r.label();
}
}
