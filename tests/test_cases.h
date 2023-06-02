/**
 * tests/test_cases.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
#include "common/clang_utils.h"
#include "common/compilation_database.h"
#include "config/config.h"
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

std::pair<clanguml::config::config, clanguml::common::compilation_database_ptr>
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

struct JsonMatcherBase : Catch::MatcherBase<nlohmann::json> {
    JsonMatcherBase(
        std::string const &operation, CasedString const &comparator);
    std::string describe() const override;

    CasedString m_comparator;
    std::string m_operation;
};

template <typename T, typename... Ts> constexpr bool has_type() noexcept
{
    return (std::is_same_v<T, Ts> || ... || false);
}

struct Public { };

struct Protected { };

struct Private { };

struct Abstract { };

struct Static { };

struct Const { };

struct Constexpr { };

struct Consteval { };

struct Noexcept { };

struct Default { };

struct Deleted { };

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

std::string _NS(std::string_view s)
{
    return fmt::format(
        "clanguml::{}::{}", Catch::getResultCapture().getCurrentTestName(), s);
}

class NamespaceWrapper {

private:
};

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
        util::replace_all(name, "[", "\\[");
        util::replace_all(name, "]", "\\]");

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

ContainsMatcher IsUnion(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("class " + str + " <<union>>", caseSensitivity));
}

ContainsMatcher IsClassTemplate(std::string const &str,
    std::string const &tmplt,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("class \"{}<{}>\"", str, tmplt), caseSensitivity));
}

ContainsMatcher IsConcept(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("class " + str + " <<concept>>", caseSensitivity));
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
            fmt::format(fmt::runtime(format_string), from, to, label),
            caseSensitivity));
    }
    else
        return ContainsMatcher(
            CasedString(fmt::format(fmt::runtime(format_string), from, to),
                caseSensitivity));
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

    return ContainsMatcher(
        CasedString(fmt::format(fmt::runtime(format_string), from, to, label),
            caseSensitivity));
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

    return ContainsMatcher(
        CasedString(fmt::format(fmt::runtime(format_string), from, to, label),
            caseSensitivity));
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

ContainsMatcher IsConstraint(std::string const &from, std::string const &to,
    std::string const &label = {},
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    if (label.empty())
        return ContainsMatcher(
            CasedString(fmt::format("{} ..> {}", from, to), caseSensitivity));
    else
        return ContainsMatcher(CasedString(
            fmt::format("{} ..> {} : {}", from, to, label), caseSensitivity));
}

ContainsMatcher IsConceptRequirement(std::string const &cpt,
    std::string const &requirement,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(requirement, caseSensitivity));
}

ContainsMatcher IsLayoutHint(std::string const &from, std::string const &hint,
    std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} -[hidden]{}- {}", from, hint, to), caseSensitivity));
}

ContainsMatcher HasComment(std::string const &comment,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("' {}", comment), caseSensitivity));
}

ContainsMatcher HasNote(std::string const &cls, std::string const &position,
    std::string const &note = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("note {} of {}", position, cls), caseSensitivity));
}

ContainsMatcher HasMemberNote(std::string const &cls, std::string const &member,
    std::string const &position, std::string const &note = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("note {} of {}::{}", position, cls, member),
            caseSensitivity));
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

    if constexpr (has_type<Constexpr, Ts...>())
        pattern += " constexpr";

    if constexpr (has_type<Consteval, Ts...>())
        pattern += " consteval";

    if constexpr (has_type<Const, Ts...>())
        pattern += " const";

    if constexpr (has_type<Abstract, Ts...>())
        pattern += " = 0";

    if constexpr (has_type<Default, Ts...>())
        pattern += " = default";

    if constexpr (has_type<Deleted, Ts...>())
        pattern += " = deleted";

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

namespace json {
struct File {
    explicit File(const std::string &f)
        : file{f}
    {
    }

    const std::string file;
};

std::optional<nlohmann::json> get_element_by_id(
    const nlohmann::json &j, const std::string &id)
{
    if (!j.contains("elements"))
        return {};

    for (const nlohmann::json &e : j["elements"]) {
        if (e["id"] == id)
            return {e};

        if (e["type"] == "namespace" || e["type"] == "folder") {
            auto maybe_e = get_element_by_id(e, id);
            if (maybe_e)
                return maybe_e;
        }
    }

    return {};
}

std::optional<nlohmann::json> get_element(
    const nlohmann::json &j, const std::string &name)
{
    if (!j.contains("elements"))
        return {};

    for (const nlohmann::json &e : j["elements"]) {
        if (e["display_name"] == name)
            return {e};

        if (e["type"] == "namespace" || e["type"] == "folder") {
            auto maybe_e = get_element(e, name);
            if (maybe_e)
                return maybe_e;
        }
    }

    return {};
}

std::optional<nlohmann::json> get_participant(
    const nlohmann::json &j, const std::string &name)
{
    if (!j.contains("participants"))
        return {};

    for (const nlohmann::json &e : j["participants"]) {
        if (e["name"] == name)
            return {e};
    }

    return {};
}

auto get_relationship(const nlohmann::json &j, const nlohmann::json &from,
    const nlohmann::json &to, const std::string &type)
{
    return std::find_if(j["relationships"].begin(), j["relationships"].end(),
        [&](const auto &it) {
            return (it["source"] == from) && (it["destination"] == to) &&
                (it["type"] == type);
        });
}

auto get_relationship(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &type)
{
    auto source = get_element(j, from);
    auto destination = get_element(j, to);

    if (!(source && destination))
        return j["relationships"].end();

    return get_relationship(j, source->at("id"), destination->at("id"), type);
}

std::string expand_name(const nlohmann::json &j, const std::string &name)
{
    if (!j.contains("using_namespace"))
        return name;

    if (name.find("::") == 0)
        return name;

    return fmt::format("{}::{}", j["using_namespace"].get<std::string>(), name);
}

bool IsClass(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "class";
}

bool IsAbstractClass(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && (e->at("type") == "class") && (e->at("is_abstract") == true);
}

bool IsClassTemplate(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "class" && e->at("is_template") == true;
}

bool IsConcept(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "concept";
}

bool IsEnum(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "enum";
}

bool IsPackage(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "namespace";
}

bool IsFolder(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, name);
    return e && e->at("type") == "folder";
}

bool IsFile(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, name);
    return e && e->at("type") == "file";
}

bool IsDeprecated(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("is_deprecated") == true;
}

bool IsBaseClass(const nlohmann::json &j, const std::string &base,
    const std::string &subclass)
{
    auto base_el = get_element(j, expand_name(j, base));
    auto subclass_el = get_element(j, expand_name(j, subclass));

    if (!base_el || !subclass_el)
        return false;

    const nlohmann::json &bases = (*subclass_el)["bases"];

    return std::find_if(bases.begin(), bases.end(), [&](const auto &it) {
        return it["id"] == base_el.value()["id"];
    }) != bases.end();
}

bool IsMethod(
    const nlohmann::json &j, const std::string &cls, const std::string &name)
{
    auto sc = get_element(j, expand_name(j, cls));

    if (!sc)
        return false;

    const nlohmann::json &methods = (*sc)["methods"];

    return std::find_if(methods.begin(), methods.end(), [&](const auto &it) {
        return it["name"] == name;
    }) != methods.end();
}

bool IsField(const nlohmann::json &j, const std::string &cls,
    const std::string &name, const std::string &type)
{
    auto sc = get_element(j, expand_name(j, cls));

    if (!sc)
        return false;

    const nlohmann::json &members = (*sc)["members"];

    return std::find_if(members.begin(), members.end(), [&](const auto &it) {
        return it["name"] == name && it["type"] == type;
    }) != members.end();
}

bool IsAssociation(nlohmann::json j, const std::string &from,
    const std::string &to, const std::string &label = "")
{
    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "association");

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && rel->at("label") != label)
        return false;

    return true;
}

bool IsComposition(nlohmann::json j, const std::string &from,
    const std::string &to, const std::string &label = "")
{
    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "composition");

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && rel->at("label") != label)
        return false;

    return true;
}

bool IsAggregation(nlohmann::json j, const std::string &from,
    const std::string &to, const std::string &label = "")
{
    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "aggregation");

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && rel->at("label") != label)
        return false;

    return true;
}

namespace detail {
bool is_dependency_impl(
    nlohmann::json j, const std::string &from, const std::string &to)
{
    auto rel = get_relationship(j, from, to, "dependency");

    return rel != j["relationships"].end();
}

} // namespace detail

bool IsDependency(
    nlohmann::json j, const std::string &from, const std::string &to)
{
    return detail::is_dependency_impl(
        j, expand_name(j, from), expand_name(j, to));
}

bool IsDependency(nlohmann::json j, const File &from, const File &to)
{
    return detail::is_dependency_impl(j, from.file, to.file);
}

bool IsInstantiation(
    nlohmann::json j, const std::string &from, const std::string &to)
{
    auto rel = get_relationship(
        j, expand_name(j, to), expand_name(j, from), "instantiation");

    return rel != j["relationships"].end();
}

bool IsFriend(nlohmann::json j, const std::string &from, const std::string &to)
{
    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "friendship");

    return rel != j["relationships"].end();
}

bool IsInnerClass(
    nlohmann::json j, const std::string &from, const std::string &to)
{
    auto rel = get_relationship(
        j, expand_name(j, to), expand_name(j, from), "containment");

    return rel != j["relationships"].end();
}

bool IsParticipant(
    const nlohmann::json &j, const std::string &name, const std::string &type)
{
    auto p = get_participant(j, expand_name(j, name));

    return p && (p->at("type") == type);
}

bool IsFunctionParticipant(const nlohmann::json &j, const std::string &name)
{
    return IsParticipant(j, name, "function");
}

bool IsClassParticipant(const nlohmann::json &j, const std::string &name)
{
    return IsParticipant(j, name, "class");
}

bool IsFileParticipant(const nlohmann::json &j, const std::string &name)
{
    return IsParticipant(j, name, "file");
}

namespace detail {
int find_message_nested(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg, const nlohmann::json &from_p,
    const nlohmann::json &to_p, int &count)
{
    const auto &messages = j["messages"];

    int res{-1};

    for (const auto &m : messages) {
        if (m.contains("branches")) {
            for (const auto &b : m["branches"]) {
                auto nested_res =
                    find_message_nested(b, from, to, msg, from_p, to_p, count);

                if (nested_res >= 0)
                    return nested_res;
            }
        }
        else if (m.contains("messages")) {
            auto nested_res =
                find_message_nested(m, from, to, msg, from_p, to_p, count);

            if (nested_res >= 0)
                return nested_res;
        }
        else {
            if ((m["from"]["participant_id"] == from_p["id"]) &&
                (m["to"]["participant_id"] == to_p["id"]) && (m["name"] == msg))
                return count;

            count++;
        }
    }

    return res;
}

int find_message_impl(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg)
{

    auto from_p = get_participant(j, from);
    auto to_p = get_participant(j, to);

    // TODO: support diagrams with multiple sequences...
    const auto &sequence_0 = j["sequences"][0];

    int count{0};

    auto res = detail::find_message_nested(
        sequence_0, from, to, msg, *from_p, *to_p, count);

    if (res >= 0)
        return res;

    throw std::runtime_error(
        fmt::format("No such message {} {} {}", from, to, msg));
}

} // namespace detail

int FindMessage(const nlohmann::json &j, const File &from, const File &to,
    const std::string &msg)
{
    return detail::find_message_impl(j, from.file, to.file, msg);
}

int FindMessage(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg)
{
    return detail::find_message_impl(
        j, expand_name(j, from), expand_name(j, to), msg);
}

} // namespace json
}
}
}
