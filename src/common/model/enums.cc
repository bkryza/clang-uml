/**
 * src/class_diagram/model/enums.cc
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
#include "enums.h"

#include <cassert>

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
        return "friendship";
    case relationship_t::kDependency:
        return "dependency";
    default:
        assert(false);
    }
}

std::string to_string(scope_t s)
{
    switch (s) {
    case scope_t::kPublic:
        return "public";
    case scope_t::kProtected:
        return "protected";
    case scope_t::kPrivate:
        return "private";
    case scope_t::kNone:
        return "none";
    default:
        assert(false);
    }
}

std::string to_string(access_t a)
{
    switch (a) {
    case access_t::kPublic:
        return "public";
    case access_t::kProtected:
        return "protected";
    case access_t::kPrivate:
        return "private";
    default:
        assert(false);
    }
}
}
