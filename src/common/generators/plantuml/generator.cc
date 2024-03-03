/**
 * @file src/common/generators/plantuml/generator.h
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
#include "generator.h"

namespace clanguml::common::generators::plantuml {

std::string to_plantuml(const relationship &r, const config::diagram &cfg)
{
    using common::model::relationship_t;

    std::string style;

    const auto &inline_style = r.style();

    if (inline_style && !inline_style.value().empty()) {
        if (inline_style && inline_style.value().back() == ']')
            style = *inline_style;
        else
            style = fmt::format("[{}]", inline_style.value());
    }

    if (style.empty() && cfg.puml) {
        if (auto config_style = cfg.puml().get_style(r.type());
            config_style.has_value()) {
            style = config_style.value();
        }
    }

    switch (r.type()) {
    case relationship_t::kOwnership:
    case relationship_t::kComposition:
        return style.empty() ? "*--" : fmt::format("*-{}-", style);
    case relationship_t::kAggregation:
        return style.empty() ? "o--" : fmt::format("o-{}-", style);
    case relationship_t::kContainment:
        return style.empty() ? "--+" : fmt::format("-{}-+", style);
    case relationship_t::kAssociation:
        return style.empty() ? "-->" : fmt::format("-{}->", style);
    case relationship_t::kInstantiation:
        return style.empty() ? "..|>" : fmt::format(".{}.|>", style);
    case relationship_t::kFriendship:
        return style.empty() ? "<.." : fmt::format("<.{}.", style);
    case relationship_t::kDependency:
        return style.empty() ? "..>" : fmt::format(".{}.>", style);
    case relationship_t::kConstraint:
        return style.empty() ? "..>" : fmt::format(".{}.>", style);
    case relationship_t::kAlias:
        return style.empty() ? ".." : fmt::format(".{}.", style);
    default:
        return "";
    }
}

std::string to_plantuml(access_t scope)
{
    switch (scope) {
    case access_t::kPublic:
        return "+";
    case access_t::kProtected:
        return "#";
    case access_t::kPrivate:
        return "-";
    default:
        return "";
    }
}

std::string to_plantuml(message_t r)
{
    switch (r) {
    case message_t::kCall:
        return "->";
    case message_t::kReturn:
        return "-->";
    default:
        return "";
    }
}

} // namespace clanguml::common::generators::plantuml
