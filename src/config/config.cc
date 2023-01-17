/**
 * src/config/config.cc
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

#include "config.h"
#include "glob/glob.hpp"

#include <filesystem>

namespace clanguml::config {

std::string to_string(const hint_t t)
{
    switch (t) {
    case hint_t::up:
        return "up";
    case hint_t::down:
        return "down";
    case hint_t::left:
        return "left";
    case hint_t::right:
        return "right";
    default:
        assert(false);
        return "";
    }
}

std::string to_string(const method_arguments ma)
{
    switch (ma) {
    case method_arguments::full:
        return "full";
    case method_arguments::abbreviated:
        return "abbreviated";
    case method_arguments::none:
        return "none";
    default:
        assert(false);
        return "";
    }
}

std::string to_string(const comment_parser_t cp)
{
    switch (cp) {
    case comment_parser_t::clang:
        return "clang";
    case comment_parser_t::plain:
        return "plain";
    default:
        assert(false);
        return "";
    }
}

std::string to_string(location_t cp)
{
    switch (cp) {
    case location_t::fileline:
        return "fileline";
    case location_t::function:
        return "function";
    case location_t::marker:
        return "marker";
    default:
        assert(false);
        return "";
    }
}

void plantuml::append(const plantuml &r)
{
    before.insert(before.end(), r.before.begin(), r.before.end());
    after.insert(after.end(), r.after.begin(), r.after.end());
}

void inheritable_diagram_options::inherit(
    const inheritable_diagram_options &parent)
{
    glob.override(parent.glob);
    using_namespace.override(parent.using_namespace);
    include_relations_also_as_members.override(
        parent.include_relations_also_as_members);
    include.override(parent.include);
    exclude.override(parent.exclude);
    puml.override(parent.puml);
    generate_method_arguments.override(parent.generate_method_arguments);
    generate_links.override(parent.generate_links);
    generate_system_headers.override(parent.generate_system_headers);
    git.override(parent.git);
    base_directory.override(parent.base_directory);
    relative_to.override(parent.relative_to);
    comment_parser.override(parent.comment_parser);
    combine_free_functions_into_file_participants.override(
        combine_free_functions_into_file_participants);
    debug_mode.override(parent.debug_mode);
}

std::string inheritable_diagram_options::simplify_template_type(
    std::string full_name) const
{
    const auto &aliases = type_aliases();

    for (const auto &[pattern, replacement] : aliases) {
        util::replace_all(full_name, pattern, replacement);
    }

    return full_name;
}

std::vector<std::string> diagram::get_translation_units(
    const std::filesystem::path &root_directory) const
{
    std::vector<std::string> translation_units{};

    for (const auto &g : glob()) {
        const auto matches = glob::glob(g, root_directory);
        for (const auto &match : matches) {
            const auto path =
                std::filesystem::canonical(root_directory / match);
            translation_units.emplace_back(path.string());
        }
    }

    return translation_units;
}

void diagram::initialize_type_aliases()
{
    if (type_aliases().count("std::basic_string<char>") == 0U) {
        type_aliases().insert({"std::basic_string<char>", "std::string"});
    }
    if (type_aliases().count("std::basic_string<char,std::char_traits<"
                             "char>,std::allocator<char>>") == 0U) {
        type_aliases().insert({"std::basic_string<char,std::char_traits<"
                               "char>,std::allocator<char>>",
            "std::string"});
    }
    if (type_aliases().count("std::basic_string<wchar_t>") == 0U) {
        type_aliases().insert({"std::basic_string<wchar_t>", "std::wstring"});
    }
    if (type_aliases().count("std::basic_string<char16_t>") == 0U) {
        type_aliases().insert(
            {"std::basic_string<char16_t>", "std::u16string"});
    }
    if (type_aliases().count("std::basic_string<char32_t>") == 0U) {
        type_aliases().insert(
            {"std::basic_string<char32_t>", "std::u32string"});
    }
    if (type_aliases().count("std::integral_constant<bool,true>") == 0U) {
        type_aliases().insert(
            {"std::integral_constant<bool,true>", "std::true_type"});
    }
    if (type_aliases().count("std::integral_constant<bool,false>") == 0U) {
        type_aliases().insert(
            {"std::integral_constant<bool,false>", "std::false_type"});
    }
}

common::model::diagram_t class_diagram::type() const
{
    return common::model::diagram_t::kClass;
}

common::model::diagram_t sequence_diagram::type() const
{
    return common::model::diagram_t::kSequence;
}

common::model::diagram_t package_diagram::type() const
{
    return common::model::diagram_t::kPackage;
}

common::model::diagram_t include_diagram::type() const
{
    return common::model::diagram_t::kInclude;
}

void class_diagram::initialize_relationship_hints()
{
    using common::model::relationship_t;

    if (relationship_hints().count("std::vector") == 0U) {
        relationship_hints().insert({"std::vector", {}});
    }
    if (relationship_hints().count("std::unique_ptr") == 0U) {
        relationship_hints().insert({"std::unique_ptr", {}});
    }
    if (relationship_hints().count("std::shared_ptr") == 0U) {
        relationship_hints().insert(
            {"std::shared_ptr", {relationship_t::kAssociation}});
    }
    if (relationship_hints().count("std::weak_ptr") == 0U) {
        relationship_hints().insert(
            {"std::weak_ptr", {relationship_t::kAssociation}});
    }
    if (relationship_hints().count("std::tuple") == 0U) {
        relationship_hints().insert({"std::tuple", {}});
    }
    if (relationship_hints().count("std::map") == 0U) {
        relationship_hint_t hint{relationship_t::kNone};
        hint.argument_hints.insert({1, relationship_t::kAggregation});
        relationship_hints().insert({"std::tuple", std::move(hint)});
    }
}

template <> void append_value<plantuml>(plantuml &l, const plantuml &r)
{
    l.append(r);
}
} // namespace clanguml::config
