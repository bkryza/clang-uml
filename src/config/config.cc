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
#include "diagram_templates.h"
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
    case hint_t::together:
        return "together";
    case hint_t::row:
        return "row";
    case hint_t::column:
        return "column";
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
    generate_packages.override(parent.generate_packages);
    package_type.override(parent.package_type);
    generate_links.override(parent.generate_links);
    generate_system_headers.override(parent.generate_system_headers);
    git.override(parent.git);
    base_directory.override(parent.base_directory);
    relative_to.override(parent.relative_to);
    comment_parser.override(parent.comment_parser);
    combine_free_functions_into_file_participants.override(
        combine_free_functions_into_file_participants);
    debug_mode.override(parent.debug_mode);
    generate_metadata.override(parent.generate_metadata);
}

std::string inheritable_diagram_options::simplify_template_type(
    std::string full_name) const
{
    type_aliases_longer_first_t aliases;
    aliases.insert(type_aliases().begin(), type_aliases().end());

    for (const auto &[pattern, replacement] : aliases) {
        util::replace_all(full_name, pattern, replacement);
    }

    return full_name;
}

std::vector<std::string> diagram::get_translation_units() const
{
    std::vector<std::string> translation_units{};

    LOG_DBG("Looking for translation units in {}",
        std::filesystem::current_path().string());

    for (const auto &g : glob()) {
        std::string glob_path =
            fmt::format("{}/{}", root_directory().string(), g.c_str());

        LOG_DBG("Searching glob path {}", glob_path);

        auto matches = glob::glob(glob_path, true, false);

        for (const auto &match : matches) {
            const auto path =
                std::filesystem::canonical(root_directory() / match);
            translation_units.emplace_back(path.string());
        }
    }

    return translation_units;
}

std::filesystem::path diagram::root_directory() const
{
    return canonical(absolute(base_directory() / relative_to()));
}

std::filesystem::path diagram::make_path_relative(
    const std::filesystem::path &p) const
{
    return relative(p, root_directory()).lexically_normal().string();
}

std::optional<std::string> diagram::get_together_group(
    const std::string &full_name) const
{
    const auto relative_name = using_namespace().relative(full_name);

    for (const auto &[hint_target, hints] : layout()) {
        for (const auto &hint : hints) {
            if (hint.hint == hint_t::together) {
                const auto &together_others =
                    std::get<std::vector<std::string>>(hint.entity);

                if ((full_name == hint_target) ||
                    util::contains(together_others, full_name))
                    return hint_target;

                if ((relative_name == hint_target) ||
                    util::contains(together_others, relative_name))
                    return hint_target;
            }
        }
    }

    return std::nullopt;
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
#if LLVM_VERSION_MAJOR >= 16
    if (type_aliases().count("std::basic_string") == 0U) {
        type_aliases().insert({"std::basic_string", "std::string"});
    }
#endif
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
