/**
 * @file tests/test_cases.h
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
#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "class_diagram/generators/mermaid/class_diagram_generator.h"
#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "class_diagram/model/diagram.h"
#include "class_diagram/visitor/translation_unit_visitor.h"
#include "common/clang_utils.h"
#include "common/compilation_database.h"
#include "common/generators/generators.h"
#include "config/config.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "include_diagram/visitor/translation_unit_visitor.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "package_diagram/visitor/translation_unit_visitor.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "sequence_diagram/visitor/translation_unit_visitor.h"
#include "test_case_utils/test_case_utils.h"
#include "util/util.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <algorithm>
#include <complex>
#include <filesystem>
#include <string>

using namespace clanguml::util;

std::pair<clanguml::config::config_ptr,
    clanguml::common::compilation_database_ptr>
load_config(const std::string &test_name);

namespace clanguml::test {

///
/// The following functions declarations define various checks on generated
/// diagrams.
/// They must be specialized for each diagram format (DiagramType) separately.
///
/// @defgroup Test Cases diagram checks
/// @{
///
// Check if generated diagram source starts with pattern
template <typename DiagramType>
bool StartsWith(const DiagramType &d, std::string pattern);

// Check if generated diagram source ends with pattern
template <typename DiagramType>
bool EndsWith(const DiagramType &d, std::string pattern);

template <typename DiagramType>
bool HasTitle(const DiagramType &d, std::string const &str);

// Check if generated diagram contains a specified enum
template <typename DiagramType>
bool IsEnum(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified union
template <typename DiagramType>
bool IsUnion(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified class
template <typename DiagramType>
bool IsClass(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified class template
template <typename DiagramType>
bool IsClassTemplate(const DiagramType &d, QualifiedName name);

template <typename DiagramType>
bool IsAbstractClassTemplate(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified abstract class
template <typename DiagramType>
bool IsAbstractClass(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified class
template <typename DiagramType>
bool IsBaseClass(
    const DiagramType &d, QualifiedName base, QualifiedName subclass);

template <typename DiagramType>
bool IsInnerClass(
    const DiagramType &d, std::string const &parent, std::string const &inner);

template <typename DiagramType, typename... Ts>
bool IsMethod(const DiagramType &d, const std::string &cls,
    const std::string &name, const std::string &type = "void",
    const std::string &params = "");

template <typename DiagramType, typename... Ts>
bool IsField(const DiagramType &d, QualifiedName cls, std::string const &name,
    std::string type = "void");

template <typename DiagramType, typename... Ts>
bool IsAssociation(const DiagramType &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "");

template <typename DiagramType, typename... Ts>
bool IsComposition(const DiagramType &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "");

template <typename DiagramType, typename... Ts>
bool IsAggregation(const DiagramType &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "");

template <typename DiagramType>
bool IsInstantiation(const DiagramType &d, std::string const &from,
    std::string const &to, std::string style = "");

template <typename DiagramType>
bool IsDependency(const DiagramType &d, QualifiedName from, QualifiedName to,
    std::string style = "");

template <typename DiagramType, typename... Ts>
bool IsFriend(
    const DiagramType &d, std::string const &from, std::string const &to);

template <typename DiagramType>
bool IsPackageDependency(
    const DiagramType &d, std::string const &from, std::string const &to);

template <typename DiagramType>
bool IsIncludeDependency(
    const DiagramType &d, std::string const &from, std::string const &to);

template <typename DiagramType>
bool IsConstraint(const DiagramType &d, QualifiedName from, QualifiedName to,
    std::string label = {}, std::string style = "");

template <typename DiagramType>
bool IsConcept(const DiagramType &d, QualifiedName cpt);

template <typename DiagramType>
bool IsConceptParameterList(
    const DiagramType &d, std::string const &cpt, std::string param);

template <typename DiagramType>
bool IsConceptRequirement(
    const DiagramType &d, std::string const &cpt, std::string requirement);

template <typename DiagramType>
bool IsLayoutHint(const DiagramType &d, std::string const &from,
    std::string const &hint, std::string const &to);

template <typename DiagramType>
bool HasComment(const DiagramType &d, std::string const &comment);

template <typename DiagramType>
bool HasNote(const DiagramType &d, std::string const &cls,
    std::string const &position, std::string const &note = "");

template <typename DiagramType>
bool HasPackageNote(const DiagramType &d, std::string const &cls,
    std::string const &position, std::string const &note = "");

template <typename DiagramType>
bool HasMessageComment(
    const DiagramType &d, std::string const &alias, std::string const &note);

template <typename DiagramType>
bool HasMemberNote(const DiagramType &d, std::string const &cls,
    std::string const &member, std::string const &position,
    std::string const &note = "");

template <typename DiagramType>
bool HasLink(const DiagramType &d, std::string const &alias,
    std::string const &link, std::string const &tooltip);

template <typename DiagramType>
bool HasMemberLink(const DiagramType &d, std::string const &method,
    std::string const &link, std::string const &tooltip);

template <typename DiagramType>
bool IsFolder(const DiagramType &d, std::string const &path);

template <typename DiagramType>
bool IsFile(const DiagramType &d, std::string const &str);

template <typename DiagramType>
bool IsSystemHeader(const DiagramType &d, std::string const &str);

template <typename DiagramType>
bool IsHeaderDependency(const DiagramType &d, std::string const &from,
    std::string const &to, std::string style = "");

template <typename DiagramType>
bool IsSystemHeaderDependency(const DiagramType &d, std::string const &from,
    std::string const &to, std::string style = "");

template <typename DiagramType, typename... Args>
bool IsNamespacePackage(const DiagramType &d, Args... args);

template <typename DiagramType, typename... Args>
bool IsDirectoryPackage(const DiagramType &d, Args... args);

template <typename DiagramType, typename... Args>
bool IsModulePackage(const DiagramType &d, Args... args);

template <typename DiagramType>
bool IsDeprecated(const DiagramType &d, std::string const &str);

template <typename DiagramType>
bool IsObjCInterface(const DiagramType &d, QualifiedName name);

template <typename DiagramType>
bool IsObjCProtocol(const DiagramType &d, QualifiedName name);

template <typename DiagramType>
bool IsObjCCategory(const DiagramType &d, QualifiedName name);

template <typename DiagramType>
int64_t FindMessage(const DiagramType &d, const Message &msg,
    int64_t offset = 0, bool fail = true);

template <typename DiagramType>
bool HasMessage(const DiagramType &d, const Message &msg)
{
    return FindMessage(d, msg, 0, false) >= 0;
}

template <typename DiagramType>
bool MessageOrder(const DiagramType &d, std::vector<Message> messages)
{
    std::vector<int64_t> order;
    int64_t offset{0};
    order.reserve(messages.size());
    std::transform(messages.begin(), messages.end(), std::back_inserter(order),
        [&d, &offset](const auto &m) {
            offset = FindMessage(d, m, offset);
            return offset;
        });
    bool are_messages_in_order = std::is_sorted(order.begin(), order.end());

    if (!are_messages_in_order) {
        FAIL(fmt::format(
            "Messages are not in order: \n[{}]", fmt::join(order, ",\n")));
        return false;
    }

    return true;
}

template <typename DiagramType>
bool MessageChainsOrder(
    const DiagramType &d, std::vector<std::vector<Message>> message_chains)
{
    // Try to match each chain to each sequence - sequence order depends
    // on platform and LLVM version
    for (const auto &message_chain : message_chains) {
        if (!MessageOrder(d, message_chain))
            return false;
    }

    return true;
}

template <typename DiagramType>
bool IsParticipant(
    const DiagramType &d, const std::string &name, const std::string &type);

template <typename DiagramType>
bool IsFunctionParticipant(const DiagramType &d, const std::string &name)
{
    return IsParticipant(d, name, "function");
}

template <typename DiagramType>
bool IsFunctionTemplateParticipant(
    const DiagramType &d, const std::string &name)
{
    return IsParticipant(d, name, "function_template");
}

template <typename DiagramType>
bool IsClassParticipant(const DiagramType &d, const std::string &name)
{
    return IsParticipant(d, name, "class");
}

template <typename DiagramType>
bool IsFileParticipant(const DiagramType &d, const std::string &name)
{
    return IsParticipant(d, name, "file");
}

///
/// @}
///

template <> bool StartsWith(const plantuml_t &d, std::string pattern)
{
    return util::starts_with(d.src, pattern);
}

template <> bool EndsWith(const plantuml_t &d, std::string pattern)
{
    return util::ends_with(d.src, pattern);
}

template <> bool HasTitle(const plantuml_t &d, std::string const &str)
{
    return d.contains("title " + str);
}

template <> bool IsEnum(const plantuml_t &d, QualifiedName enm)
{
    return d.contains(
        fmt::format("enum {}", d.get_alias(enm.str(d.generate_packages))));
}

template <> bool IsUnion(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(fmt::format(
        "class {} <<union>>", d.get_alias(cls.str(d.generate_packages))));
}

template <> bool IsClass(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("class {}", d.get_alias(cls.str(d.generate_packages))));
}

template <> bool IsObjCInterface(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("class {}", d.get_alias(cls.str(d.generate_packages))));
}

template <> bool IsObjCProtocol(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("protocol {}", d.get_alias(cls.str(d.generate_packages))));
}

template <> bool IsObjCCategory(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("class {}", d.get_alias(cls.str(d.generate_packages))));
}

template <> bool IsClassTemplate(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("class \"{}\"", cls.str(d.generate_packages)));
}

template <> bool IsAbstractClassTemplate(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("abstract \"{}\"", cls.str(d.generate_packages)));
}

template <> bool IsAbstractClass(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("abstract {}", d.get_alias(cls.str(d.generate_packages))));
}

template <>
bool IsBaseClass(
    const plantuml_t &d, QualifiedName base, QualifiedName subclass)
{
    return d.contains(
        fmt::format("{} <|-- {}", d.get_alias(base.str(d.generate_packages)),
            d.get_alias(subclass.str(d.generate_packages))));
}

template <>
bool IsInnerClass(
    const plantuml_t &d, std::string const &parent, std::string const &inner)
{
    return d.contains(d.get_alias(inner) + " --+ " + d.get_alias(parent));
}

template <typename... Ts>
bool IsMethod(const plantuml_t &d, std::string const &cls,
    std::string const &name, std::string const &type = "void",
    std::string const &params = "")
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

    if constexpr (has_type<Coroutine, Ts...>())
        pattern += " [coroutine]";

    pattern += " : " + type;

    return d.contains(pattern);
}

template <typename... Ts>
bool IsField(const plantuml_t &d, QualifiedName cls, std::string const &name,
    std::string type)
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

    return d.contains(pattern + " : " + type);
}

template <typename... Ts>
bool IsAssociation(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    auto from_id = d.get_alias(from);
    auto to_id = d.get_alias(to);

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" -{}->", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {}";

    if (!label.empty()) {
        std::string label_prefix;
        if constexpr (has_type<Public, Ts...>())
            label_prefix = "+";
        else if constexpr (has_type<Protected, Ts...>())
            label_prefix = "#";
        else
            label_prefix = "-";

        format_string += " : {}{}";
        return d.contains(fmt::format(
            fmt::runtime(format_string), from_id, to_id, label_prefix, label));
    }

    return d.contains(fmt::format(fmt::runtime(format_string), from_id, to_id));
}

template <typename... Ts>
bool IsComposition(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" *-{}-", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}{}";

    return d.contains(fmt::format(fmt::runtime(format_string),
        d.get_alias(from), d.get_alias(to), label_prefix, label));
}

template <typename... Ts>
bool IsAggregation(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" o-{}-", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}{}";

    return d.contains(fmt::format(fmt::runtime(format_string),
        d.get_alias(from), d.get_alias(to), label_prefix, label));
}

template <>
bool IsInstantiation(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    return d.contains(
        fmt::format("{} .{}.|> {}", d.get_alias(to), style, d.get_alias(from)));
}

template <>
bool IsDependency(const plantuml_t &d, QualifiedName from, QualifiedName to,
    std::string style)
{
    return d.contains(fmt::format(
               "{} .{}.> {}", d.get_alias(from), style, d.get_alias(to))) ||
        d.contains(fmt::format("{} .{}.> {}", d.get_alias(from.name), style,
            d.get_alias(to.name)));
}

template <typename... Ts>
bool IsFriend(
    const plantuml_t &d, std::string const &from, std::string const &to)
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    return d.contains(fmt::format("{} <.. {} : {}<<friend>>", d.get_alias(from),
        d.get_alias(to), pattern));
}

template <>
bool IsPackageDependency(
    const plantuml_t &d, std::string const &from, std::string const &to)
{
    return d.contains(
        fmt::format("{} .{}.> {}", d.get_alias(from), "", d.get_alias(to)));
}

template <>
bool IsIncludeDependency(
    const plantuml_t &d, std::string const &from, std::string const &to)
{
    return d.contains(
        fmt::format("{} .{}.> {}", d.get_alias(from), "", d.get_alias(to)));
}

template <>
bool IsConstraint(const plantuml_t &d, QualifiedName from, QualifiedName to,
    std::string label, std::string style)
{
    if (label.empty())
        return d.contains(fmt::format("{} .{}.> {}", d.get_alias(from.name),
            style, d.get_alias(to.name)));

    return d.contains(fmt::format("{} .{}.> {} : {}", d.get_alias(from.name),
        style, d.get_alias(to.name), label));
}

template <> bool IsConcept(const plantuml_t &d, QualifiedName cpt)
{
    return d.contains("class " + d.get_alias(cpt) + " <<concept>>") ||
        d.contains("class " + d.get_alias(cpt.name) + " <<concept>>");
}

template <>
bool IsConceptRequirement(
    const plantuml_t &d, std::string const &cpt, std::string requirement)
{
    return d.contains(requirement);
}

template <>
bool IsConceptParameterList(
    const plantuml_t &d, std::string const &cpt, std::string params)
{
    return d.contains(params);
}

template <>
bool IsLayoutHint(const plantuml_t &d, std::string const &from,
    std::string const &hint, std::string const &to)
{
    return d.contains(fmt::format(
        "{} -[hidden]{}- {}", d.get_alias(from), hint, d.get_alias(to)));
}

template <> bool HasComment(const plantuml_t &d, std::string const &comment)
{
    return d.contains(fmt::format("' {}", comment));
}

template <>
bool HasNote(const plantuml_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    return d.contains(fmt::format("note {} of {}", position, d.get_alias(cls)));
}

template <>
bool HasMemberNote(const plantuml_t &d, std::string const &cls,
    std::string const &member, std::string const &position,
    std::string const &note)
{
    return d.contains(
        fmt::format("note {} of {}::{}", position, d.get_alias(cls), member));
}

template <>
bool HasPackageNote(const plantuml_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    return d.contains(fmt::format("note {} of {}", position, d.get_alias(cls)));
}

template <>
bool HasLink(const plantuml_t &d, std::string const &element,
    std::string const &link, std::string const &tooltip)
{
    return d.contains(
        fmt::format("{} [[{}{{{}}}]]", d.get_alias(element), link, tooltip));
}

template <>
bool HasMemberLink(const plantuml_t &d, std::string const &method,
    std::string const &link, std::string const &tooltip)
{
    return d.contains(fmt::format("{} [[[{}{{{}}}]]]", method, link, tooltip));
}

template <> bool IsFolder(const plantuml_t &d, std::string const &path)
{
    const std::filesystem::path p{path};
    return d.contains("folder \"" + p.filename().string() + "\"");
}

template <> bool IsFile(const plantuml_t &d, std::string const &path)
{
    const std::filesystem::path p{path};
    return d.contains("file \"" + p.filename().string() + "\"");
}

template <> bool IsSystemHeader(const plantuml_t &d, std::string const &path)
{
    return d.contains("file \"" + path + "\"");
}

template <>
bool IsHeaderDependency(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    return d.contains(
        fmt::format("{} --> {}", d.get_alias(util::split(from, "/").back()),
            d.get_alias(util::split(to, "/").back())));
}

template <>
bool IsSystemHeaderDependency(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    return d.contains(
        fmt::format("{} ..> {}", d.get_alias(util::split(from, "/").back()),
            d.get_alias(util::split(to, "/").back())));
}

template <typename... Args> auto get_last(Args &&...args)
{
    return std::get<sizeof...(Args) - 1>(std::forward_as_tuple(args...));
}

template <typename... Args>
bool IsNamespacePackage(const plantuml_t &d, Args... args)
{
    const auto &name = get_last(args...);
    return d.contains("package [" + name + "]");
}

template <typename... Args>
bool IsDirectoryPackage(const plantuml_t &d, Args... args)
{
    const auto &name = get_last(args...);
    return d.contains("package [" + name + "]");
}

template <typename... Args>
bool IsModulePackage(const plantuml_t &d, Args... args)
{
    const auto &name = get_last(args...);
    return d.contains("package [" + name + "]");
}

template <> bool IsDeprecated(const plantuml_t &d, const std::string &name)
{
    return d.contains(d.get_alias(name) + " <<deprecated>> ");
}

template <>
int64_t FindMessage(
    const plantuml_t &d, const Message &msg, int64_t offset, bool fail)
{
    auto msg_str = msg.message;
    util::replace_all(msg_str, "(", "\\(");
    util::replace_all(msg_str, ")", "\\)");
    util::replace_all(msg_str, "*", "\\*");
    util::replace_all(msg_str, "[", "\\[");
    util::replace_all(msg_str, "]", "\\]");
    util::replace_all(msg_str, "+", "\\+");

    if (msg.is_cuda_kernel)
        msg_str = fmt::format("<< CUDA Kernel >>\\\\n{}", msg_str);
    if (msg.is_cuda_device)
        msg_str = fmt::format("<< CUDA Device >>\\\\n{}", msg_str);

    std::string style;
    if (msg.is_static)
        style = "__";

    std::string call_pattern{"__INVALID__"};

    if (msg.is_entrypoint) {
        call_pattern =
            fmt::format("\\[-> {} : {}", d.get_alias(msg.to), msg_str);
    }
    else if (msg.is_exitpoint) {
        call_pattern = fmt::format("\\[<-- {}", d.get_alias(msg.to));
    }
    else if (msg.is_incontrolcondition) {
        call_pattern = fmt::format(
            "{} {} {} "
            "(\\[\\[.*\\]\\] )?: \\*\\*\\[\\*\\*{}{}{}\\*\\*\\]\\*\\*",
            d.get_alias(msg.from), "->", d.get_alias(msg.to), style, msg_str,
            style);
    }
    else if (msg.is_response) {
        call_pattern = fmt::format("{} {} {} : //{}//", d.get_alias(msg.from),
            "-->", d.get_alias(msg.to), msg_str);
    }
    else {
        call_pattern = fmt::format("{} {} {} "
                                   "(\\[\\[.*\\]\\] )?: {}{}{}",
            d.get_alias(msg.from), "->", d.get_alias(msg.to), style, msg_str,
            style);
    }

    auto match_offset = d.find(call_pattern, offset);

    if (match_offset < 0) {
        if (fail)
            FAIL(fmt::format("Missing message: {} -> {} {} ({})",
                msg.from.str(), msg.to.str(), msg.message, call_pattern));
        return -1;
    }

    return match_offset + offset;
}

template <>
bool HasMessageComment(const plantuml_t &d, std::string const &participant,
    std::string const &note)
{
    std::string note_escaped{note};
    util::replace_all(note_escaped, "(", "\\(");
    util::replace_all(note_escaped, ")", "\\)");

    return d.search(fmt::format("note over {}\\n{}\\nend note",
        d.get_alias(participant), note_escaped));
}

template <>
bool IsParticipant(
    const plantuml_t &d, const std::string &name, const std::string &type)
{
    return d.contains(
        fmt::format("participant \"{}\" as ", name, d.get_alias(name)));
}

//
// MermaidJS test helpers
//

template <> bool HasTitle(const mermaid_t &d, std::string const &str)
{
    return d.contains("title: " + str);
}

template <> bool IsEnum(const mermaid_t &d, QualifiedName enm)
{
    return d.search(std::string("class ") + d.get_alias(enm) +
        " \\{\\n\\s+<<enumeration>>");
}

template <> bool IsUnion(const mermaid_t &d, QualifiedName cls)
{
    return d.search(
        std::string("class ") + d.get_alias(cls) + " \\{\\n\\s+<<union>>");
}

template <> bool IsClass(const mermaid_t &d, QualifiedName cls)
{
    return d.contains(fmt::format("class {}", d.get_alias(cls)));
}

template <> bool IsObjCInterface(const mermaid_t &d, QualifiedName cls)
{
    return d.contains(fmt::format("class {}", d.get_alias(cls)));
}

template <> bool IsObjCProtocol(const mermaid_t &d, QualifiedName cls)
{
    return d.contains(fmt::format("class {}", d.get_alias(cls)));
}

template <> bool IsObjCCategory(const mermaid_t &d, QualifiedName cls)
{
    return d.contains(fmt::format("class {}", d.get_alias(cls)));
}

template <> bool IsClassTemplate(const mermaid_t &d, QualifiedName cls)
{
    return d.contains(fmt::format("class {}", d.get_alias(cls)));
}

template <> bool IsAbstractClassTemplate(const mermaid_t &d, QualifiedName cls)
{
    return d.search(
        std::string("class ") + d.get_alias(cls) + " \\{\\n\\s+<<abstract>>");
}

template <> bool IsAbstractClass(const mermaid_t &d, QualifiedName name)
{
    return d.search(
        std::string("class ") + d.get_alias(name) + " \\{\\n\\s+<<abstract>>");
}

template <>
bool IsBaseClass(const mermaid_t &d, QualifiedName base, QualifiedName subclass)
{
    return d.contains(
        fmt::format("{} <|-- {}", d.get_alias(base), d.get_alias(subclass)));
}

template <>
bool IsInnerClass(
    const mermaid_t &d, std::string const &parent, std::string const &inner)
{
    return d.contains(d.get_alias(parent) + " ()-- " + d.get_alias(inner));
}

template <typename... Ts>
bool IsMethod(const mermaid_t &d, std::string const &cls,
    std::string const &name, std::string type = "void",
    std::string const &params = "")
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    auto params_escaped =
        clanguml::common::generators::mermaid::escape_name(params);

    pattern += "(" + params_escaped + ")";

    std::vector<std::string> method_mods;
    if constexpr (has_type<Default, Ts...>())
        method_mods.push_back("default");
    if constexpr (has_type<Const, Ts...>())
        method_mods.push_back("const");
    if constexpr (has_type<Constexpr, Ts...>())
        method_mods.push_back("constexpr");
    if constexpr (has_type<Consteval, Ts...>())
        method_mods.push_back("consteval");
    if constexpr (has_type<Coroutine, Ts...>())
        method_mods.push_back("coroutine");

    pattern += " : ";

    if (!method_mods.empty()) {
        pattern += fmt::format("[{}] ", fmt::join(method_mods, ","));
    }

    auto type_escaped =
        clanguml::common::generators::mermaid::escape_name(type);

    pattern += type_escaped;

    if constexpr (has_type<Abstract, Ts...>())
        pattern += "*";

    if constexpr (has_type<Static, Ts...>())
        pattern += "$";

    return d.contains(pattern);
}

template <typename... Ts>
bool IsField(const mermaid_t &d, QualifiedName cls, std::string const &name,
    std::string type)
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

    util::replace_all(type, "<", "&lt;");
    util::replace_all(type, ">", "&gt;");
    util::replace_all(type, "(", "&lpar;");
    util::replace_all(type, ")", "&rpar;");
    util::replace_all(type, "##", "::");
    util::replace_all(type, "{", "&lbrace;");
    util::replace_all(type, "}", "&rbrace;");

    return d.contains(pattern + " : " + type);
}

template <typename... Ts>
bool IsAssociation(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    auto from_id = d.get_alias(from);
    auto to_id = d.get_alias(to);

    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += " -->";

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {}";

    if (!label.empty()) {
        format_string += " : {}{}";
        return d.contains(fmt::format(
            fmt::runtime(format_string), from_id, to_id, label_prefix, label));
    }

    return d.contains(fmt::format(fmt::runtime(format_string), from_id, to_id));
}

template <typename... Ts>
bool IsComposition(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" *--", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}{}";

    return d.contains(fmt::format(fmt::runtime(format_string),
        d.get_alias(from), d.get_alias(to), label_prefix, label));
}

template <typename... Ts>
bool IsAggregation(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += " o--";

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}{}";

    return d.contains(fmt::format(fmt::runtime(format_string),
        d.get_alias(from), d.get_alias(to), label_prefix, label));
}

template <>
bool IsInstantiation(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    return d.contains(
        fmt::format("{} ..|> {}", d.get_alias(to), d.get_alias(from)));
}

template <>
bool IsDependency(
    const mermaid_t &d, QualifiedName from, QualifiedName to, std::string style)
{
    if (d.diagram_type == common::model::diagram_t::kClass) {
        return d.contains(
            fmt::format("{} ..> {}", d.get_alias(from), d.get_alias(to)));
    }

    return d.contains(
        fmt::format("{} -.-> {}", d.get_alias(from), d.get_alias(to)));
}

template <>
bool IsHeaderDependency(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    return d.contains(
        fmt::format("{} --> {}", d.get_alias(util::split(from, "/").back()),
            d.get_alias(util::split(to, "/").back())));
}

template <>
bool IsSystemHeaderDependency(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    return d.contains(
        fmt::format("{} -.-> {}", d.get_alias(util::split(from, "/").back()),
            d.get_alias(util::split(to, "/").back())));
}

template <typename... Ts>
bool IsFriend(
    const mermaid_t &d, std::string const &from, std::string const &to)
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    return d.contains(fmt::format(
        "{} <.. {} : {}[friend]", d.get_alias(from), d.get_alias(to), pattern));
}

template <>
bool IsPackageDependency(
    const mermaid_t &d, std::string const &from, std::string const &to)
{
    return d.contains(
        fmt::format("{} -.-> {}", d.get_alias(from), "", d.get_alias(to)));
}

template <>
bool IsIncludeDependency(
    const mermaid_t &d, std::string const &from, std::string const &to)
{
    return d.contains(
        fmt::format("{} -.-> {}", d.get_alias(from), "", d.get_alias(to)));
}

template <>
bool IsConstraint(const mermaid_t &d, QualifiedName from, QualifiedName to,
    std::string label, std::string style)
{
    auto from_id = d.get_alias(from);
    auto to_id = d.get_alias(to);

    if (label.empty())
        return d.contains(fmt::format("{} ..> {}", from_id, "", to_id));

    util::replace_all(label, "<", "&lt;");
    util::replace_all(label, ">", "&gt;");
    util::replace_all(label, "(", "&lpar;");
    util::replace_all(label, ")", "&rpar;");
    util::replace_all(label, "##", "::");
    util::replace_all(label, "{", "&lbrace;");
    util::replace_all(label, "}", "&rbrace;");

    if (label.empty())
        return d.contains(fmt::format("{} ..> {}", from_id, to_id));

    return d.contains(fmt::format("{} ..> {} : {}", from_id, to_id, label));
}

template <> bool IsConcept(const mermaid_t &d, QualifiedName cpt)
{
    return d.search(
        std::string("class ") + d.get_alias(cpt) + " \\{\\n\\s+<<concept>>");
}

template <>
bool IsConceptRequirement(
    const mermaid_t &d, std::string const &cpt, std::string requirement)
{
    util::replace_all(requirement, "<", "&lt;");
    util::replace_all(requirement, ">", "&gt;");
    util::replace_all(requirement, "##", "::");
    util::replace_all(requirement, "{", "&lbrace;");
    util::replace_all(requirement, "}", "&rbrace;");

    return d.contains(requirement);
}

template <>
bool IsConceptParameterList(
    const mermaid_t &d, std::string const &cpt, std::string params)
{
    util::replace_all(params, "<", "&lt;");
    util::replace_all(params, ">", "&gt;");
    util::replace_all(params, "##", "::");
    util::replace_all(params, "{", "&lbrace;");
    util::replace_all(params, "}", "&rbrace;");

    return d.contains(params);
}

template <>
bool IsLayoutHint(const mermaid_t &d, std::string const &from,
    std::string const &hint, std::string const &to)
{
    return true;
}

template <> bool HasComment(const mermaid_t &d, std::string const &comment)
{
    return d.contains(fmt::format("%% {}", comment));
}

template <>
bool HasNote(const mermaid_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    if (d.diagram_type == common::model::diagram_t::kPackage) {
        return d.contains(fmt::format("-.- {}", d.get_alias(cls)));
    }

    return d.contains(fmt::format("note for {}", d.get_alias(cls)));
}

template <>
bool HasMemberNote(const mermaid_t &d, std::string const &cls,
    std::string const &member, std::string const &position,
    std::string const &note)
{
    return d.contains(
        fmt::format("note for {} \"{}\"", d.get_alias(cls), note));
}

template <>
bool HasPackageNote(const mermaid_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    return d.contains(fmt::format("-.- {}", d.get_alias(cls)));
}

template <>
bool HasLink(const mermaid_t &d, std::string const &element,
    std::string const &link, std::string const &tooltip)
{
    return d.contains(fmt::format(
        "click {} href \"{}\" \"{}\"", d.get_alias(element), link, tooltip));
}

template <>
bool HasMemberLink(const mermaid_t &d, std::string const &method,
    std::string const &link, std::string const &tooltip)
{
    return true;
}

template <> bool IsFolder(const mermaid_t &d, std::string const &path)
{
    return d.contains("subgraph " + d.get_alias(util::split(path, "/").back()));
}

template <> bool IsFile(const mermaid_t &d, std::string const &path)
{
    return d.contains(d.get_alias(util::split(path, "/").back()) + "[");
}

template <> bool IsSystemHeader(const mermaid_t &d, std::string const &path)
{
    return d.contains(d.get_alias(path) + "[");
}

template <typename... Args>
bool IsNamespacePackage(const mermaid_t &d, Args... args)
{
    if (d.diagram_type == class_diagram::model::diagram_t::kClass) {
        std::vector<std::string> toks{{args...}};
        return d.contains(fmt::format("[\"{}", fmt::join(toks, "::")));
    }

    const auto &name = get_last(args...);
    return d.contains(fmt::format("subgraph {}", d.get_alias(name)));
}

template <typename... Args>
bool IsDirectoryPackage(const mermaid_t &d, Args... args)
{
    if (d.diagram_type == class_diagram::model::diagram_t::kClass) {
        // MermaidJS does not support packages in class diagrams
        return true;
    }

    const auto &name = get_last(args...);
    return d.contains("subgraph " + d.get_alias(name));
}

template <typename... Args>
bool IsModulePackage(const mermaid_t &d, Args... args)
{
    if (d.diagram_type == class_diagram::model::diagram_t::kClass) {
        // MermaidJS does not support packages in class diagrams
        return true;
    }

    const auto &name = get_last(args...);
    return d.contains("subgraph " + d.get_alias(name));
}

template <> bool IsDeprecated(const mermaid_t &d, const std::string &name)
{
    return d.contains(d.get_alias(name));
}

template <>
int64_t FindMessage(
    const mermaid_t &d, const Message &msg, int64_t offset, bool fail)
{
    auto msg_str = msg.message;

    util::replace_all(msg_str, "(", "\\(");
    util::replace_all(msg_str, ")", "\\)");
    util::replace_all(msg_str, "*", "\\*");
    util::replace_all(msg_str, "[", "\\[");
    util::replace_all(msg_str, "]", "\\]");
    util::replace_all(msg_str, "+", "\\+");

    if (msg.is_cuda_kernel)
        msg_str = fmt::format("<< CUDA Kernel >><br>{}", msg_str);
    if (msg.is_cuda_device)
        msg_str = fmt::format("<< CUDA Device >><br>{}", msg_str);

    std::string call_pattern{"__INVALID__"};

    if (msg.is_entrypoint) {
        call_pattern =
            fmt::format("\\* ->> {} : {}", d.get_alias(msg.to), msg_str);
    }
    else if (msg.is_exitpoint) {
        call_pattern = fmt::format("{} -->> \\*", d.get_alias(msg.to), msg_str);
    }
    else if (msg.is_incontrolcondition) {
        call_pattern = fmt::format("{} {} {} : \\[{}\\]", d.get_alias(msg.from),
            "->>", d.get_alias(msg.to), msg_str);
    }
    else if (msg.is_response) {
        call_pattern = fmt::format("{} {} {} : {}", d.get_alias(msg.from),
            "-->>", d.get_alias(msg.to), msg_str);
    }
    else {
        call_pattern = fmt::format("{} {} {} : {}", d.get_alias(msg.from),
            "->>", d.get_alias(msg.to), msg_str);
    }

    auto match_offset = d.find(call_pattern, offset);

    if (match_offset < 0) {
        if (fail)
            FAIL(fmt::format("Missing message: {} -> {} {} ({})",
                msg.from.str(), msg.to.str(), msg.message, call_pattern));
        return -1;
    }

    return match_offset + offset;
}

template <>
bool HasMessageComment(
    const mermaid_t &d, std::string const &participant, std::string const &note)
{
    std::string note_escaped{note};
    util::replace_all(note_escaped, "\\n", "<br/>");

    return d.contains(std::string("note over ") + d.get_alias(participant) +
        ": " + note_escaped);
}

template <>
bool IsParticipant(
    const mermaid_t &d, const std::string &name, const std::string &type)
{
    return d.contains(fmt::format("participant {}", d.get_alias(name)));
}

template <> bool HasTitle(const json_t &d, std::string const &str)
{
    return d.src.contains("title") && d.src["title"] == str;
}

template <> bool IsAbstractClass(const json_t &d, QualifiedName cls)
{
    auto e =
        get_element(d.src, expand_name(d.src, cls.str(d.generate_packages)));
    return e && e->at("type") == "class" && e->at("is_abstract");
}

template <> bool IsEnum(const json_t &d, QualifiedName enm)
{
    auto e =
        get_element(d.src, expand_name(d.src, enm.str(d.generate_packages)));
    return e && e->at("type") == "enum";
}

template <> bool IsUnion(const json_t &d, QualifiedName enm)
{
    auto e =
        get_element(d.src, expand_name(d.src, enm.str(d.generate_packages)));
    return e && e->at("type") == "class" && e->at("is_union");
}

template <> bool IsClass(const json_t &d, QualifiedName cls)
{
    auto e = get_element(
        d.src, expand_name(d.src, cls.str(d.generate_packages)), "class");
    return e && e->at("type") == "class" && !e->at("is_abstract");
}

template <> bool IsObjCInterface(const json_t &d, QualifiedName cls)
{
    auto e = get_element(d.src,
        expand_name(d.src, cls.str(d.generate_packages)), "objc_interface");
    return e && e->at("type") == "objc_interface";
}

template <> bool IsObjCProtocol(const json_t &d, QualifiedName cls)
{
    auto e = get_element(d.src,
        expand_name(d.src, cls.str(d.generate_packages)), "objc_protocol");
    return e && e->at("type") == "objc_protocol";
}

template <> bool IsObjCCategory(const json_t &d, QualifiedName cls)
{
    auto e = get_element(d.src,
        expand_name(d.src, cls.str(d.generate_packages)), "objc_category");
    return e && e->at("type") == "objc_category";
}

template <> bool IsClassTemplate(const json_t &d, QualifiedName cls)
{
    auto e =
        get_element(d.src, expand_name(d.src, cls.str(d.generate_packages)));
    return e && e->at("type") == "class" && e->at("is_template");
}

template <> bool IsAbstractClassTemplate(const json_t &d, QualifiedName cls)
{
    auto e =
        get_element(d.src, expand_name(d.src, cls.str(d.generate_packages)));
    return e && e->at("type") == "class" && e->at("is_abstract");
}

template <>
bool IsBaseClass(const json_t &d, QualifiedName base, QualifiedName subclass)
{
    const auto &j = d.src;
    auto base_el =
        get_element(j, expand_name(j, base.str(d.generate_packages)));
    auto subclass_el =
        get_element(j, expand_name(j, subclass.str(d.generate_packages)));

    if (!base_el || !subclass_el)
        return false;

    const nlohmann::json &bases = (*subclass_el)["bases"];

    return std::find_if(bases.begin(), bases.end(), [&](const auto &it) {
        return it["id"] == base_el.value()["id"];
    }) != bases.end();
}

template <>
bool IsInnerClass(
    const json_t &d, std::string const &parent, std::string const &inner)
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, inner), expand_name(j, parent), "containment");

    return rel != j["relationships"].end();
}

template <typename... Ts>
bool IsMethod(const json_t &d, const std::string &cls, std::string const &name,
    std::string type = "void", std::string const &params = "")
{
    const auto &j = d.src;
    auto sc = get_element(j, expand_name(j, cls));

    if (!sc)
        return false;

    const nlohmann::json &methods = (*sc)["methods"];

    return std::find_if(methods.begin(), methods.end(), [name](const auto &it) {
        return it["display_name"] == name;
    }) != methods.end();
}

template <typename... Ts>
bool IsField(const json_t &d, QualifiedName cls, std::string const &name,
    std::string type)
{
    const auto &j = d.src;

    auto sc = get_element(j, expand_name(j, cls.str(d.generate_packages)));

    if (!sc)
        return false;

    const nlohmann::json &members = (*sc)["members"];

    return std::find_if(members.begin(), members.end(), [&](const auto &it) {
        return it["name"] == name && it["type"] == type;
    }) != members.end();
}

template <typename... Ts>
bool IsAssociation(const json_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "association", label);

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && (rel->at("label") != label))
        return false;

    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    if (rel->at("access") != access)
        return false;

    return true;
}

template <typename... Ts>
bool IsComposition(const json_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "composition", label);

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && (rel->at("label") != label))
        return false;

    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    if (rel->at("access") != access)
        return false;

    return true;
}

template <typename... Ts>
bool IsAggregation(const json_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "aggregation", label);

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && (rel->at("label") != label))
        return false;

    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    if (rel->at("access") != access)
        return false;

    return true;
}

template <>
bool IsInstantiation(const json_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, to), expand_name(j, from), "instantiation");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <>
bool IsDependency(
    const json_t &d, QualifiedName from, QualifiedName to, std::string style)
{
    const auto &j = d.src;

    auto rel =
        get_relationship(j, expand_name(j, from.str(d.generate_packages)),
            expand_name(j, to.str(d.generate_packages)), "dependency");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <typename... Ts>
bool IsFriend(const json_t &d, std::string const &from, std::string const &to)
{
    std::string access;

    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "friendship");

    return rel != j["relationships"].end() && rel->at("access") == access;
}

template <>
bool IsPackageDependency(
    const json_t &d, std::string const &from, std::string const &to)
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "dependency");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <>
bool IsIncludeDependency(
    const json_t &d, std::string const &from, std::string const &to)
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "dependency");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <> bool IsConcept(const json_t &d, QualifiedName cpt)
{
    const auto &j = d.src;

    auto e = get_element(j, expand_name(j, cpt.str(d.generate_packages)));
    return e && e->at("type") == "concept";
}

template <>
bool IsConstraint(const json_t &d, QualifiedName from, QualifiedName to,
    std::string label, std::string style)
{
    const auto &j = d.src;

    auto rel =
        get_relationship(j, expand_name(j, from.str(d.generate_packages)),
            expand_name(j, to.str(d.generate_packages)), "constraint", label);

    if (rel == j["relationships"].end())
        return false;

    if ((*rel)["label"] != label)
        return false;

    return true;
}

template <>
bool IsConceptRequirement(
    const json_t &d, std::string const &cpt, std::string requirement)
{
    const auto &j = d.src;

    auto e = get_element(j, expand_name(j, cpt));

    if (!e)
        return false;

    const nlohmann::json &statements = (*e)["statements"];

    return std::find_if(statements.begin(), statements.end(),
               [requirement](const auto &it) { return it == requirement; }) !=
        statements.end();
}

template <>
bool IsConceptParameterList(
    const json_t &d, std::string const &cpt, std::string parameter_list)
{
    const auto &j = d.src;

    auto e = get_element(j, expand_name(j, cpt));

    if (!e)
        return false;

    const nlohmann::json &parameters = (*e)["parameters"];

    std::vector<std::string> params;
    for (const auto &it : parameters) {
        params.push_back(fmt::format("{} {}", it["type"].get<std::string>(),
            it["name"].get<std::string>()));
    }

    return parameter_list == fmt::format("({})", fmt::join(params, ","));
}

template <>
bool IsLayoutHint(const json_t &d, std::string const &from,
    std::string const &hint, std::string const &to)
{
    return true;
}

template <> bool HasComment(const json_t &d, std::string const &comment)
{
    // Comments are not included in JSON
    return true;
}

template <>
bool HasNote(const json_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    const auto &j = d.src;

    auto sc = get_element(j, expand_name(j, cls));

    if (!sc)
        return false;

    std::string formatted = (*sc)["comment"]["formatted"];

    return util::contains(formatted, note);
}

template <>
bool HasPackageNote(const json_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    return true;
}

template <>
bool HasMemberNote(const json_t &d, std::string const &cls,
    std::string const &member, std::string const &position,
    std::string const &note)
{
    return true;
}

template <>
bool HasLink(const json_t &d, std::string const &alias, std::string const &link,
    std::string const &tooltip)
{
    return true;
}

template <>
bool HasMemberLink(const json_t &d, std::string const &method,
    std::string const &link, std::string const &tooltip)
{
    return true;
}

template <> bool IsFolder(const json_t &d, std::string const &path)
{
    const auto &j = d.src;

    auto e = get_element(j, path);
    return e && e->at("type") == "folder";
}

template <> bool IsFile(const json_t &d, std::string const &path)
{
    const auto &j = d.src;

    auto e = get_element(j, path);
    return e && e->at("type") == "file";
}

template <> bool IsSystemHeader(const json_t &d, std::string const &path)
{
    const auto &j = d.src;

    auto e = get_element(j, path);
    return e && e->at("type") == "file" && e->at("file_kind") == "header" &&
        e->at("is_system");
}

template <>
bool IsHeaderDependency(const json_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "association");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <>
bool IsSystemHeaderDependency(const json_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "dependency");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <typename PackageT, typename... Args>
bool IsPackagePath(
    const nlohmann::json &j, const std::string &head, Args... args)
{
    if constexpr (sizeof...(Args) == 0) {
        auto e = get_element(j, expand_name(j, head));

        return e && e->at("type") == package_type_name<PackageT>();
    }
    else {
        auto e = get_element(j, head);
        if (!e.has_value())
            return false;

        return IsPackagePath<PackageT>(*e, args...);
    }
}

template <typename... Args>
bool IsNamespacePackage(const json_t &d, Args... args)
{
    const auto &j = d.src;

    return IsPackagePath<NamespacePackage>(j, std::forward<Args>(args)...);
}

template <typename... Args>
bool IsDirectoryPackage(const json_t &d, Args... args)
{
    const auto &j = d.src;

    return IsPackagePath<DirectoryPackage>(j, std::forward<Args>(args)...);
}

template <typename... Args> bool IsModulePackage(const json_t &d, Args... args)
{
    const auto &j = d.src;

    return IsPackagePath<ModulePackage>(j, std::forward<Args>(args)...);
}

template <> bool IsDeprecated(const json_t &d, const std::string &name)
{
    const auto &j = d.src;

    auto e = get_element(j, expand_name(j, name));
    return e && e->at("is_deprecated") == true;
}

template <>
int64_t FindMessage(
    const json_t &d, const Message &msg, int64_t offset, bool fail)
{
    if (msg.is_response) {
        // TODO: Currently response are not generated as separate messages
        //       in JSON format
        return offset;
    }

    if (msg.is_entrypoint || msg.is_exitpoint)
        return offset;

    try {
        return json_helpers::find_message(d.src, msg.from.str(), msg.to.str(),
            msg.message, msg.return_type, offset);
    }
    catch (std::exception &e) {
        if (!fail)
            return -1;

        std::cout << "FindMessage failed with error: " << e.what() << "\n";

        throw e;
    }
}

template <>
bool MessageChainsOrder<json_t>(
    const json_t &d, std::vector<std::vector<Message>> message_chains)
{
    const auto sequence_chains_count{
        d.src["sequences"][0]["message_chains"].size()};

    for (const auto &messages : message_chains) {
        for (uint32_t chain_index = 0; chain_index < sequence_chains_count;
             chain_index++) {
            int64_t offset{0};

            std::vector<int64_t> order;
            order.reserve(messages.size());
            std::transform(messages.begin(), messages.end(),
                std::back_inserter(order),
                [&d, &offset, chain_index](const auto &m) -> int64_t {
                    try {
                        offset = find_message_in_chain(
                            d, m, offset, true, chain_index);
                        return offset;
                    }
                    catch (...) {
                        return 0;
                    }
                });

            bool are_messages_in_order =
                std::is_sorted(order.begin(), order.end());

            if (are_messages_in_order)
                return true;
        }
    }

    FAIL(fmt::format("Messages are not in order"));

    return false;
}

template <>
bool HasMessageComment(
    const json_t &d, std::string const &alias, std::string const &note)
{
    return true;
}

template <>
bool IsParticipant(
    const json_t &d, const std::string &name, const std::string &type)
{
    const auto &j = d.src;

    auto p = get_participant(j, expand_name(j, name));

    return p && (p->at("type") == type);
}

template <> bool HasTitle(const graphml_t &d, std::string const &str)
{
    auto query = fmt::format("/graphml/desc[text()='{}']", str);
    auto title_node = d.src.select_node(query.c_str()).node();
    return !!title_node;
}

template <> bool IsClass(const graphml_t &d, QualifiedName cls)
{
    auto class_node = get_element(d, "class", cls);

    return !!class_node;
}

template <> bool IsEnum(const graphml_t &d, QualifiedName cls)
{
    auto class_node = get_element(d, "enum", cls);

    return !!class_node;
}

template <> bool IsUnion(const graphml_t &d, QualifiedName cls)
{
    auto class_node = get_element(d, "class", cls);
    if (!class_node)
        return false;

    const auto node_stereotype_id = get_attr_key_id(d, "node", "stereotype");

    auto query =
        fmt::format("data[@key='{}' and text()='union']", node_stereotype_id);

    auto class_abstract_stereotype_node =
        class_node.node().select_node(query.c_str());

    return !!class_abstract_stereotype_node;
}

template <> bool IsAbstractClass(const graphml_t &d, QualifiedName cls)
{
    auto class_node = get_element(d, "class", cls);
    if (!class_node)
        return false;

    const auto node_stereotype_id = get_attr_key_id(d, "node", "stereotype");

    auto query = fmt::format(
        "data[@key='{}' and text()='abstract']", node_stereotype_id);

    auto class_abstract_stereotype_node =
        class_node.node().select_node(query.c_str());

    return !!class_abstract_stereotype_node;
}

template <> bool IsAbstractClassTemplate(const graphml_t &d, QualifiedName cls)
{
    return IsAbstractClass(d, cls);
}

template <> bool IsClassTemplate(const graphml_t &d, QualifiedName cls)
{
    pugi::xpath_node class_node = get_element(d, "class", cls);

    return !!class_node &&
        has_data(d, class_node.node(), "is_template", "true");
}

template <> bool IsObjCProtocol(const graphml_t &d, QualifiedName cls)
{
    auto class_node = get_element(d, "objc_protocol", cls);

    return !!class_node;
}

template <> bool IsObjCCategory(const graphml_t &d, QualifiedName cls)
{
    auto class_node = get_element(d, "objc_category", cls);

    return !!class_node;
}

template <> bool IsObjCInterface(const graphml_t &d, QualifiedName cls)
{
    auto class_node = get_element(d, "objc_interface", cls);

    return !!class_node;
}

template <> bool IsConcept(const graphml_t &d, QualifiedName cpt)
{
    pugi::xpath_node concept_node = get_element(d, "concept", cpt);

    return !!concept_node;
}

template <>
bool IsConceptRequirement(
    const graphml_t &d, std::string const &cpt, std::string requirement)
{
    return true;
}

template <>
bool IsConstraint(const graphml_t &d, QualifiedName from, QualifiedName to,
    std::string label, std::string style)
{
    return true;
}

template <>
bool IsConceptParameterList(
    const graphml_t &d, std::string const &cpt, std::string parameter_list)
{
    return true;
}

template <>
bool IsBaseClass(const graphml_t &d, QualifiedName base, QualifiedName subclass)
{
    auto base_node =
        get_element(d, {"class", "objc_interface", "objc_protocol"}, base);
    auto subclass_node =
        get_element(d, {"class", "objc_interface", "objc_protocol"}, subclass);

    if (!base_node || !subclass_node)
        return false;

    return find_relationship(d,
        subclass_node.node().attribute("id").as_string(),
        base_node.node().attribute("id").as_string(), "extension");
}

template <>
bool IsInnerClass(
    const graphml_t &d, std::string const &parent, std::string const &inner)
{
    auto from_node = get_element(d, QualifiedName{parent});
    auto to_node = get_element(d, QualifiedName{inner});

    if (!from_node || !to_node)
        return false;

    return find_relationship(d, to_node.node().attribute("id").as_string(),
        from_node.node().attribute("id").as_string(), "containment");
}

template <typename... Ts>
bool IsAssociation(const graphml_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    auto from_node_id =
        get_element(d, QualifiedName{from}).node().attribute("id").as_string();
    auto to_node_id =
        get_element(d, QualifiedName{to}).node().attribute("id").as_string();

    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    auto edge_node = get_relationship(
        d, from_node_id, to_node_id, "association", label, access);

    return !!edge_node;
}

template <>
bool IsDependency(
    const graphml_t &d, QualifiedName from, QualifiedName to, std::string style)
{
    auto from_node = get_element(d, from);
    auto to_node = get_element(d, to);

    if (!from_node || !to_node)
        return false;

    return find_relationship(d, from_node.node().attribute("id").as_string(),
        to_node.node().attribute("id").as_string(), "dependency");
}

template <>
bool IsInstantiation(const graphml_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    auto from_node = get_element(d, QualifiedName{from});
    auto to_node = get_element(d, QualifiedName{to});

    if (!from_node || !to_node)
        return false;

    return find_relationship(d, to_node.node().attribute("id").as_string(),
        from_node.node().attribute("id").as_string(), "instantiation");
}

template <typename... Ts>
bool IsAggregation(const graphml_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    auto from_node = get_element(d, QualifiedName{from});
    auto to_node = get_element(d, QualifiedName{to});

    if (!from_node || !to_node)
        return false;

    return !!get_relationship(d, from_node.node().attribute("id").as_string(),
        to_node.node().attribute("id").as_string(), "aggregation", label,
        access);
}

template <typename... Ts>
bool IsComposition(const graphml_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    auto from_node = get_element(d, QualifiedName{from});
    auto to_node = get_element(d, QualifiedName{to});

    if (!from_node || !to_node)
        return false;

    return !!get_relationship(d, from_node.node().attribute("id").as_string(),
        to_node.node().attribute("id").as_string(), "composition", label,
        access);
}

template <typename... Ts>
bool IsFriend(
    const graphml_t &d, std::string const &from, std::string const &to)
{
    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    auto from_node = get_element(d, QualifiedName{from});
    auto to_node = get_element(d, QualifiedName{to});

    if (!from_node || !to_node)
        return false;

    return !!get_relationship(d, from_node.node().attribute("id").as_string(),
        to_node.node().attribute("id").as_string(), "friendship", "", access);
}

template <>
bool HasNote(const graphml_t &d, std::string const &element,
    std::string const &position, std::string const &note)
{
    if (note.empty())
        return true;

    auto element_node = get_element(d, QualifiedName{element}).node();

    const auto node_name_id = get_attr_key_id(d, "node", "name");
    const auto node_type_id = get_attr_key_id(d, "node", "type");

    const auto select_all_notes =
        fmt::format("//node[data[@key='{}' and text()='note']]", node_type_id);

    auto note_results = d.src.select_nodes(select_all_notes.c_str());
    for (const auto &note_result : note_results) {
        if (has_data(d, note_result.node(), "name", note) ||
            has_data(d, note_result.node(), "name", util::ltrim(note))) {
            return true;
        }
    }

    return false;
}

template <>
bool HasLink(const graphml_t &d, std::string const &element,
    std::string const &link, std::string const &tooltip)
{
    auto element_node = get_element(d, QualifiedName{element}).node();

    return has_data(d, element_node, "url", link) &&
        has_data(d, element_node, "tooltip", tooltip);
}

template <>
bool HasMemberLink(const graphml_t &d, std::string const &method,
    std::string const &link, std::string const &tooltip)
{
    return true;
}

template <typename... Ts>
bool IsMethod(const graphml_t &d, const std::string &cls,
    std::string const &name, std::string type = "void",
    std::string const &params = "")
{
    return true;
}

template <typename... Ts>
bool IsField(const graphml_t &d, QualifiedName cls, std::string const &name,
    std::string type)
{
    return true;
}

template <>
bool IsLayoutHint(const graphml_t &d, std::string const &from,
    std::string const &hint, std::string const &to)
{
    return true;
}

template <typename PackageT, typename... Args>
bool IsPackagePath(const graphml_t &d, const pugi::xml_node &parent,
    const std::string &head, Args... args)
{
    if constexpr (sizeof...(Args) == 0) {
        auto subgraph = get_graph(d, parent, head);
        return !!subgraph;
    }
    else {
        auto subgraph = get_graph(d, parent, head);
        if (!subgraph)
            return false;

        return IsPackagePath<PackageT>(d, subgraph.node(), args...);
    }
}

template <typename... Args>
bool IsNamespacePackage(const graphml_t &d, Args... args)
{
    const auto &graphml = d.src.child("graphml").child("graph");

    return IsPackagePath<NamespacePackage>(
        d, graphml, std::forward<Args>(args)...);
}

template <typename... Args>
bool IsDirectoryPackage(const graphml_t &d, Args... args)
{
    const auto &graphml = d.src.child("graphml").child("graph");

    return IsPackagePath<DirectoryPackage>(
        d, graphml, std::forward<Args>(args)...);
}

template <typename... Args>
bool IsModulePackage(const graphml_t &d, Args... args)
{
    const auto &graphml = d.src.child("graphml").child("graph");

    return IsPackagePath<ModulePackage>(
        d, graphml, std::forward<Args>(args)...);
}

template <> bool HasComment(const graphml_t &d, std::string const &comment)
{
    // Comments are not included in GraphML
    return true;
}

template <> bool IsDeprecated(const graphml_t &d, const std::string &name)
{
    auto node = get_element(d, QualifiedName{name});
    if (!node)
        return false;

    const auto node_stereotype_id = get_attr_key_id(d, "node", "stereotype");

    auto query = fmt::format(
        "data[@key='{}' and text()='deprecated']", node_stereotype_id);

    auto class_abstract_stereotype_node =
        node.node().select_node(query.c_str());

    return !!class_abstract_stereotype_node;
}

template <>
bool IsPackageDependency(
    const graphml_t &d, std::string const &from, std::string const &to)
{
    auto from_node = get_element(d, QualifiedName{from});
    auto to_node = get_element(d, QualifiedName{to});

    if (!from_node || !to_node)
        return false;

    return !!get_relationship(d, from_node.node().attribute("id").as_string(),
        to_node.node().attribute("id").as_string(), "dependency");
}

template <> bool IsFolder(const graphml_t &d, std::string const &path)
{
    std::filesystem::path path_fs{path};

    assert(path_fs.is_relative());

    std::vector<std::string> p{path_fs.begin(), path_fs.end()};

    const auto node = get_nested_element(d, p);

    if (!node)
        return false;

    return !!has_data(d, node, "type", "folder");
}

template <> bool IsFile(const graphml_t &d, std::string const &path)
{
    auto file_node = get_file_node(
        d, d.src.child("graphml").child("graph"), std::filesystem::path{path});

    if (!file_node)
        return false;

    return !!has_data(d, file_node, "type", "file");
}

template <> bool IsSystemHeader(const graphml_t &d, std::string const &path)
{
    auto file_node = get_element(d, QualifiedName{path}).node();

    if (!file_node)
        return false;

    return !!has_data(d, file_node, "type", "file") &&
        !!has_data(d, file_node, "stereotype", "header") &&
        !has_data(d, file_node, "stereotype", "source") &&
        !!has_data(d, file_node, "is_system", "true");
}

template <>
bool IsHeaderDependency(const graphml_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    auto from_node = get_file_node(
        d, d.src.child("graphml").child("graph"), std::filesystem::path{from});
    auto to_node = get_file_node(
        d, d.src.child("graphml").child("graph"), std::filesystem::path{to});

    if (!from_node || !to_node)
        return false;

    return find_relationship(d, from_node.attribute("id").as_string(),
        to_node.attribute("id").as_string(), "association");
}

template <>
bool IsSystemHeaderDependency(const graphml_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    auto from_node = get_file_node(
        d, d.src.child("graphml").child("graph"), std::filesystem::path{from});
    auto to_node = get_element(d, QualifiedName{to});

    if (!from_node || !to_node)
        return false;

    return find_relationship(d, from_node.attribute("id").as_string(),
        to_node.node().attribute("id").as_string(), "dependency");
}

} // namespace clanguml::test
