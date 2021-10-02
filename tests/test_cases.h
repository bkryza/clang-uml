/**
 * tests/test_cases.h
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
#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "config/config.h"
#include "cx/compilation_database.h"
#include "puml/class_diagram_generator.h"
#include "puml/sequence_diagram_generator.h"
#include "uml/class_diagram/model/diagram.h"
#include "uml/class_diagram_visitor.h"
#include "uml/sequence_diagram_visitor.h"
#include "util/util.h"

#define CATCH_CONFIG_RUNNER

#include "catch.h"

#include <complex>
#include <filesystem>
#include <string>

using Catch::Matchers::Contains;
using Catch::Matchers::EndsWith;
using Catch::Matchers::StartsWith;
using Catch::Matchers::VectorContains;
using clanguml::cx::compilation_database;
using namespace clanguml::util;

std::pair<clanguml::config::config, cppast::libclang_compilation_database>
load_config(const std::string &test_name);
std::pair<clanguml::config::config, compilation_database> load_config2(
    const std::string &test_name);

clanguml::model::sequence_diagram::diagram generate_sequence_diagram(
    compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram);

clanguml::class_diagram::model::diagram generate_class_diagram(
    compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram);

std::string generate_sequence_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::model::sequence_diagram::diagram &model);

std::string generate_class_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::class_diagram::model::diagram &model);

void save_puml(const std::string &path, const std::string &puml);

namespace clanguml {
namespace test {
namespace matchers {

using Catch::CaseSensitive;
using Catch::Matchers::StdString::CasedString;
using Catch::Matchers::StdString::ContainsMatcher;

template <typename T, typename... Ts> constexpr bool has_type() noexcept
{
    return (std::is_same_v<T, Ts> || ... || false);
}

struct Public {
};

struct Protected {
};

struct Private {
};

struct Abstract {
};

struct Static {
};

struct Const {
};

struct Default {
};

struct HasCallWithResultMatcher : ContainsMatcher {
    HasCallWithResultMatcher(
        CasedString const &comparator, CasedString const &resultComparator)
        : ContainsMatcher(comparator)
        , m_resultComparator{resultComparator}
    {
    }

    bool match(std::string const &source) const override
    {
        return Catch::contains(
                   m_comparator.adjustString(source), m_comparator.m_str) &&
            Catch::contains(
                m_comparator.adjustString(source), m_resultComparator.m_str);
    }

    CasedString m_resultComparator;
};

ContainsMatcher HasCall(std::string const &from, std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("\"{}\" -> \"{}\" : {}()", from, from, message),
            caseSensitivity));
}

ContainsMatcher HasCall(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("\"{}\" -> \"{}\" : {}()", from, to, message),
            caseSensitivity));
}

auto HasCallWithResponse(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCallWithResultMatcher(
        CasedString(fmt::format("\"{}\" -> \"{}\" : {}()", from, to, message),
            caseSensitivity),
        CasedString(
            fmt::format("\"{}\" --> \"{}\"", to, from), caseSensitivity));
}

struct AliasMatcher {
    AliasMatcher(const std::string &puml_)
        : puml{split(puml_, "\n")}
    {
    }

    std::string operator()(const std::string &name)
    {
        std::vector<std::string> patterns;
        patterns.push_back("class \"" + name + "\" as ");
        patterns.push_back("abstract \"" + name + "\" as ");
        patterns.push_back("enum \"" + name + "\" as ");

        for (const auto &line : puml) {
            for (const auto &pattern : patterns) {
                const auto idx = line.find(pattern);
                if (idx != std::string::npos) {
                    std::string res = line.substr(idx + pattern.size());
                    return trim(res);
                }
            }
        }

        return "__INVALID__ALIAS__";
    }

    const std::vector<std::string> puml;
};

ContainsMatcher IsClass(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("class " + str, caseSensitivity));
}

ContainsMatcher IsClassTemplate(std::string const &str,
    std::string const &tmplt,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("class \"{}<{}>\"", str, tmplt), caseSensitivity));
}

ContainsMatcher IsEnum(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("enum " + str, caseSensitivity));
}

ContainsMatcher IsAbstractClass(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("abstract " + str, caseSensitivity));
}

ContainsMatcher IsBaseClass(std::string const &base, std::string const &sub,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(base + " <|-- " + sub, caseSensitivity));
}

ContainsMatcher IsInnerClass(std::string const &parent,
    std::string const &inner,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(inner + " --+ " + parent, caseSensitivity));
}

ContainsMatcher IsAssociation(std::string const &from, std::string const &to,
    std::string const &label, std::string multiplicity_source = "",
    std::string multiplicity_dest = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += " -->";

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}";

    return ContainsMatcher(CasedString(
        fmt::format(format_string, from, to, label), caseSensitivity));
}

ContainsMatcher IsFriend(std::string const &from, std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} <.. {} : <<friend>>", from, to), caseSensitivity));
}

ContainsMatcher IsComposition(std::string const &from, std::string const &to,
    std::string const &label, std::string multiplicity_source = "",
    std::string multiplicity_dest = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += " *--";

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}";

    return ContainsMatcher(CasedString(
        fmt::format(format_string, from, to, label), caseSensitivity));
}

ContainsMatcher IsAggregation(std::string const &from, std::string const &to,
    std::string const &label, std::string multiplicity_source = "",
    std::string multiplicity_dest = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += " o--";

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}";

    return ContainsMatcher(CasedString(
        fmt::format(format_string, from, to, label), caseSensitivity));
}

ContainsMatcher IsAggregationWithStyle(std::string const &from,
    std::string const &to, std::string const &label, std::string style,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("{} o-[{}]- {} : {}", from, style, to, label),
            caseSensitivity));
}

ContainsMatcher IsAssociationWithStyle(std::string const &from,
    std::string const &to, std::string const &label, std::string style,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("{} -[{}]-> {} : {}", from, style, to, label),
            caseSensitivity));
}

ContainsMatcher IsCompositionWithStyle(std::string const &from,
    std::string const &to, std::string const &label, std::string style,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("{} *-[{}]- {} : {}", from, style, to, label),
            caseSensitivity));
}

ContainsMatcher IsInstantiation(std::string const &from, std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("{} ..|> {}", to, from), caseSensitivity));
}

ContainsMatcher IsDependency(std::string const &from, std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("{} ..> {}", from, to), caseSensitivity));
}

ContainsMatcher HasNote(std::string const &cls, std::string const &position,
    std::string const &note,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("note {} of {}", position, cls), caseSensitivity));
}

template <typename... Ts>
ContainsMatcher IsMethod(std::string const &name,
    std::string const &type = "void",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string pattern;
    if constexpr (has_type<Static, Ts...>())
        pattern += "{static} ";

    if constexpr (has_type<Abstract, Ts...>())
        pattern += "{abstract} ";

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    pattern += "()";

    if constexpr (has_type<Const, Ts...>())
        pattern += " const";

    if constexpr (has_type<Abstract, Ts...>())
        pattern += " = 0";

    pattern += " : " + type;

    return ContainsMatcher(CasedString(pattern, caseSensitivity));
}

template <typename... Ts>
ContainsMatcher IsField(std::string const &name,
    std::string const &type = "void",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string pattern;
    if constexpr (has_type<Static, Ts...>())
        pattern += "{static} ";

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    return ContainsMatcher(
        CasedString(pattern + " : " + type, caseSensitivity));
}
}
}
}
