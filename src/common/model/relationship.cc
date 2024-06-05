/**
 * @file src/common/model/relationship.cc
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

#include "relationship.h"

#include <utility>

namespace clanguml::common::model {

relationship::relationship(relationship_t type, eid_t destination,
    access_t access, std::string label, std::string multiplicity_source,
    std::string multiplicity_destination)
    : type_{type}
    , destination_{destination}
    , multiplicity_source_{std::move(multiplicity_source)}
    , multiplicity_destination_{std::move(multiplicity_destination)}
    , label_{std::move(label)}
    , access_{access}
{
}

void relationship::set_type(relationship_t type) noexcept { type_ = type; }

relationship_t relationship::type() const noexcept { return type_; }

void relationship::set_destination(eid_t destination)
{
    destination_ = destination;
}

eid_t relationship::destination() const { return destination_; }

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

void relationship::set_access(access_t access) noexcept { access_ = access; }

access_t relationship::access() const noexcept { return access_; }

bool operator==(const relationship &l, const relationship &r)
{
    return l.type() == r.type() && l.destination() == r.destination() &&
        l.label() == r.label();
}
} // namespace clanguml::common::model
