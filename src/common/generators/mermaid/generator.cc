/**
 * @file src/common/generators/mermaid/generator.h
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

namespace clanguml::common::generators::mermaid {

std::string to_mermaid(relationship_t r)
{
    switch (r) {
    case relationship_t::kOwnership:
    case relationship_t::kComposition:
        return "*--";
    case relationship_t::kAggregation:
        return "o--";
    case relationship_t::kContainment:
        return "()--";
    case relationship_t::kAssociation:
        return "-->";
    case relationship_t::kInstantiation:
        return "..|>";
    case relationship_t::kFriendship:
        return "<..";
    case relationship_t::kDependency:
        return "..>";
    case relationship_t::kConstraint:
        return "..>";
    case relationship_t::kAlias:
        return "..";
    default:
        return "";
    }
}

std::string to_mermaid(access_t scope)
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

std::string to_mermaid(message_t r)
{
    switch (r) {
    case message_t::kCall:
        return "->>";
    case message_t::kReturn:
        return "-->>";
    default:
        return "";
    }
}

std::string indent(const unsigned level)
{
    const auto kIndentWidth = 4UL;
    return std::string(level * kIndentWidth, ' '); // NOLINT
}

std::string render_name(std::string name, bool round_brackets)
{
    util::replace_all(name, "<", "&lt;");
    util::replace_all(name, ">", "&gt;");
    if (round_brackets) {
        util::replace_all(name, "(", "&lpar;");
        util::replace_all(name, ")", "&rpar;");
    }
    util::replace_all(name, "##", "::");
    util::replace_all(name, "{", "&lbrace;");
    util::replace_all(name, "}", "&rbrace;");

    return name;
}
} // namespace clanguml::common::generators::mermaid
