/**
 * src/uml/class_diagram_model.cc
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

#include "class_diagram_model.h"

namespace clanguml {
namespace model {
namespace class_diagram {
std::atomic_uint64_t element::m_nextId = 1;

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
}
}
}
