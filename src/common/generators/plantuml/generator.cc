/**
 * src/common/generators/plantuml/generator.h
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
#include "generator.h"

namespace clanguml::common::generators::plantuml {

std::string to_plantuml(relationship_t r, std::string style)
{
    switch (r) {
    case relationship_t::kOwnership:
    case relationship_t::kComposition:
        return style.empty() ? "*--" : fmt::format("*-[{}]-", style);
    case relationship_t::kAggregation:
        return style.empty() ? "o--" : fmt::format("o-[{}]-", style);
    case relationship_t::kContainment:
        return style.empty() ? "--+" : fmt::format("-[{}]-+", style);
    case relationship_t::kAssociation:
        return style.empty() ? "-->" : fmt::format("-[{}]->", style);
    case relationship_t::kInstantiation:
        return style.empty() ? "..|>" : fmt::format(".[{}].|>", style);
    case relationship_t::kFriendship:
        return style.empty() ? "<.." : fmt::format("<.[{}].", style);
    case relationship_t::kDependency:
        return style.empty() ? "..>" : fmt::format(".[{}].>", style);
    default:
        return "";
    }
}

std::string to_plantuml(scope_t scope)
{
    switch (scope) {
    case scope_t::kPublic:
        return "+";
    case scope_t::kProtected:
        return "#";
    case scope_t::kPrivate:
        return "-";
    default:
        return "";
    }
}

}
