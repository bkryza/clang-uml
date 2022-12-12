/**
 * tests/test_cases.h
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
#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "class_diagram/model/diagram.h"
#include "class_diagram/visitor/translation_unit_visitor.h"
#include "config/config.h"
#include "cx/util.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "include_diagram/visitor/translation_unit_visitor.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "package_diagram/visitor/translation_unit_visitor.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "sequence_diagram/visitor/translation_unit_visitor.h"
#include "util/util.h"

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_CONSOLE_WIDTH 512

#include "catch.h"

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <algorithm>
#include <complex>
#include <filesystem>
#include <string>

using Catch::Matchers::Contains;
using Catch::Matchers::EndsWith;
using Catch::Matchers::Equals;
using Catch::Matchers::Matches;
using Catch::Matchers::StartsWith;
using Catch::Matchers::VectorContains;

using namespace clanguml::util;

std::pair<clanguml::config::config,
    std::unique_ptr<clang::tooling::CompilationDatabase>>
load_config(const std::string &test_name);

std::string generate_sequence_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::sequence_diagram::model::diagram &model);

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
using Catch::Matchers::StdString::RegexMatcher;

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

template <typename T> class HasCallMatcher : public Catch::MatcherBase<T> {
    T m_from, m_to, m_message;

public:
    HasCallMatcher(T from, T to, T message)
        : m_from(from)
        , m_to{to}
        , m_message{message}
    {
        util::replace_all(m_message, "(", "\\(");
        util::replace_all(m_message, ")", "\\)");
        util::replace_all(m_message, "*", "\\*");
        util::replace_all(m_message, "[", "\\[");
        util::replace_all(m_message, "]", "\\]");
    }

    bool match(T const &in) const override
    {
        std::istringstream fin(in);
        std::string line;
        std::regex r{fmt::format("{} -> {} "
                                 "(\\[\\[.*\\]\\] )?: {}",
            m_from, m_to, m_message)};
        while (std::getline(fin, line)) {
            std::smatch base_match;
            std::regex_search(in, base_match, r);
            if (base_match.size() > 0)
                return true;
        }

        return false;
    }

    std::string describe() const override
    {
        std::ostringstream ss;
        ss << "has call "
           << fmt::format("{} -> {} : {}", m_from, m_to, m_message);
        return ss.str();
    }
};

auto HasCall(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCallMatcher(from, to, message);
}

auto HasCallInControlCondition(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCallMatcher(from, to, fmt::format("**[**{}**]**", message));
}

auto HasCall(std::string const &from, std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCall(from, from, message, caseSensitivity);
}

auto HasCallWithResponse(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
               fmt::format("{} --> {}", to, from), caseSensitivity)) &&
        HasCallMatcher(from, to, message);
}

ContainsMatcher HasEntrypoint(std::string const &to, std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("[-> {} : {}", to, message), caseSensitivity));
}

ContainsMatcher HasExitpoint(std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("[<-- {}", to), caseSensitivity));
}

struct AliasMatcher {
    AliasMatcher(const std::string &puml_)
        : puml{split(puml_, "\n")}
    {
    }

    std::string operator()(std::string name)
    {
        std::vector<std::regex> patterns;

        const std::string alias_regex("([A-Z]_[0-9]+)");

        util::replace_all(name, "(", "\\(");
        util::replace_all(name, ")", "\\)");
        util::replace_all(name, " ", "\\s");
        util::replace_all(name, "*", "\\*");

        patterns.push_back(
            std::regex{"class\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"abstract\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"enum\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"package\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"package\\s\\[" + name + "\\]\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"file\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"folder\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"participant\\s\"" + name + "\"\\sas\\s" + alias_regex});

        std::smatch base_match;

        for (const auto &line : puml) {
            for (const auto &pattern : patterns) {
                if (std::regex_search(line, base_match, pattern) &&
                    base_match.size() == 2) {
                    std::ssub_match base_sub_match = base_match[1];
                    std::string alias = base_sub_match.str();
                    return trim(alias);
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

ContainsMatcher IsAbstractClassTemplate(std::string const &str,
    std::string const &tmplt,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("abstract \"{}<{}>\"", str, tmplt), caseSensitivity));
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
    std::string const &label = "", std::string multiplicity_source = "",
    std::string multiplicity_dest = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += " -->";

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {}";

    if (!label.empty()) {
        format_string += " : {}";
        return ContainsMatcher(CasedString(
            fmt::format(format_string, from, to, label), caseSensitivity));
    }
    else
        return ContainsMatcher(
            CasedString(fmt::format(format_string, from, to), caseSensitivity));
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

ContainsMatcher IsLayoutHint(std::string const &from, std::string const &hint,
    std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} -[hidden]{}- {}", from, hint, to), caseSensitivity));
}

ContainsMatcher HasNote(std::string const &cls, std::string const &position,
    std::string const &note = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("note {} of {}", position, cls), caseSensitivity));
}

ContainsMatcher HasLink(std::string const &alias, std::string const &link,
    std::string const &tooltip,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} [[{}{{{}}}]]", alias, link, tooltip), caseSensitivity));
}

template <typename... Ts>
ContainsMatcher IsMethod(std::string const &name,
    std::string const &type = "void", std::string const &params = "",
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

    pattern += "(" + params + ")";

    if constexpr (has_type<Const, Ts...>())
        pattern += " const";

    if constexpr (has_type<Abstract, Ts...>())
        pattern += " = 0";

    if constexpr (has_type<Default, Ts...>())
        pattern += " = default";

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

template <typename... Ts>
ContainsMatcher IsFriend(std::string const &from, std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    return ContainsMatcher(
        CasedString(fmt::format("{} <.. {} : {}<<friend>>", from, to, pattern),
            caseSensitivity));
}

ContainsMatcher IsPackage(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("package [" + str + "]", caseSensitivity));
}

ContainsMatcher IsFolder(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("folder \"" + str + "\"", caseSensitivity));
}

ContainsMatcher IsFile(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("file \"" + str + "\"", caseSensitivity));
}

ContainsMatcher IsDeprecated(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(str + " <<deprecated>> ", caseSensitivity));
}
}
}
}
