/**
 * @file src/config/config.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

std::string to_string(element_filter_t::filtered_type ft)
{
    switch (ft) {
    case element_filter_t::filtered_type::any:
        return "any";
    case element_filter_t::filtered_type::class_:
        return "class";
    case element_filter_t::filtered_type::function:
        return "function";
    case element_filter_t::filtered_type::method:
        return "method";
    case element_filter_t::filtered_type::member:
        return "member";
    case element_filter_t::filtered_type::enum_:
        return "enum";
    case element_filter_t::filtered_type::concept_:
        return "concept";
    case element_filter_t::filtered_type::package:
        return "package";
    case element_filter_t::filtered_type::function_template:
        return "function_template";
    case element_filter_t::filtered_type::objc_method:
        return "objc_method";
    case element_filter_t::filtered_type::objc_member:
        return "objc_member";
    case element_filter_t::filtered_type::objc_protocol:
        return "objc_protocol";
    case element_filter_t::filtered_type::objc_category:
        return "objc_category";
    case element_filter_t::filtered_type::objc_interface:
        return "objc_interface";
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

std::string to_string(method_type mt)
{
    switch (mt) {
    case method_type::constructor:
        return "constructor";
    case method_type::destructor:
        return "destructor";
    case method_type::assignment:
        return "assignment";
    case method_type::operator_:
        return "operator";
    case method_type::defaulted:
        return "defaulted";
    case method_type::deleted:
        return "deleted";
    case method_type::static_:
        return "static";
    }

    assert(false);
    return "";
}

std::string to_string(callee_type mt)
{
    switch (mt) {
    case callee_type::constructor:
        return "constructor";
    case callee_type::assignment:
        return "assignment";
    case callee_type::operator_:
        return "operator";
    case callee_type::defaulted:
        return "defaulted";
    case callee_type::static_:
        return "static";
    case callee_type::method:
        return "method";
    case callee_type::function:
        return "function";
    case callee_type::function_template:
        return "function_template";
    case callee_type::lambda:
        return "lambda";
    case callee_type::cuda_kernel:
        return "cuda_kernel";
    case callee_type::cuda_device:
        return "cuda_device";
    }

    assert(false);
    return "";
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

std::string to_string(package_type_t pt)
{
    switch (pt) {
    case package_type_t::kNamespace:
        return "namespace";
    case package_type_t::kDirectory:
        return "directory";
    case package_type_t::kModule:
        return "module";
    default:
        assert(false);
        return "";
    }
}

std::string to_string(member_order_t mo)
{
    switch (mo) {
    case member_order_t::lexical:
        return "lexical";
    case member_order_t::as_is:
        return "as_is";
    default:
        assert(false);
        return "";
    }
}
std::string to_string(context_direction_t cd)
{
    switch (cd) {
    case context_direction_t::inward:
        return "inward";
    case context_direction_t::outward:
        return "outward";
    case context_direction_t::any:
        return "any";
    default:
        assert(false);
        return "";
    }
}

std::string to_string(filter_mode_t fm)
{
    switch (fm) {
    case filter_mode_t::basic:
        return "basic";
    case filter_mode_t::advanced:
        return "advanced";
    default:
        assert(false);
        return "";
    }
}

std::optional<std::string> plantuml::get_style(
    const common::model::relationship_t relationship_type) const
{
    if (style.count(to_string(relationship_type)) == 0)
        return {};

    return style.at(to_string(relationship_type));
}

std::optional<std::string> plantuml::get_style(
    const std::string &element_type) const
{
    if (style.count(element_type) == 0)
        return {};

    return style.at(element_type);
}

void plantuml::append(const plantuml &r)
{
    before.insert(before.end(), r.before.begin(), r.before.end());
    after.insert(after.end(), r.after.begin(), r.after.end());
    if (cmd.empty())
        cmd = r.cmd;
}

void mermaid::append(const mermaid &r)
{
    before.insert(before.end(), r.before.begin(), r.before.end());
    after.insert(after.end(), r.after.begin(), r.after.end());
    if (cmd.empty())
        cmd = r.cmd;
}

void graphml::append(const graphml &r)
{
    auto tmp = r.notes;
    notes.merge(tmp);
}

std::filesystem::path inheritable_diagram_options::root_directory() const
{
    return weakly_canonical(absolute(base_directory() / relative_to()));
}

void inheritable_diagram_options::inherit(
    const inheritable_diagram_options &parent)
{
    glob.override(parent.glob);
    using_namespace.override(parent.using_namespace);
    using_module.override(parent.using_module);
    include_relations_also_as_members.override(
        parent.include_relations_also_as_members);
    filter_mode.override(parent.filter_mode);
    include_system_headers.override(parent.include_system_headers);
    include.override(parent.include);
    exclude.override(parent.exclude);
    puml.override(parent.puml);
    mermaid.override(parent.mermaid);
    graphml.override(parent.graphml);
    generate_method_arguments.override(parent.generate_method_arguments);
    fold_repeated_activities.override(parent.fold_repeated_activities);
    generate_concept_requirements.override(
        parent.generate_concept_requirements);
    generate_packages.override(parent.generate_packages);
    generate_template_argument_dependencies.override(
        parent.generate_template_argument_dependencies);
    package_type.override(parent.package_type);
    generate_template_argument_dependencies.override(
        parent.generate_template_argument_dependencies);
    skip_redundant_dependencies.override(parent.skip_redundant_dependencies);
    generate_links.override(parent.generate_links);
    generate_system_headers.override(parent.generate_system_headers);
    git.override(parent.git);
    base_directory.override(parent.base_directory);
    relative_to.override(parent.relative_to);
    comment_parser.override(parent.comment_parser);
    combine_free_functions_into_file_participants.override(
        parent.combine_free_functions_into_file_participants);
    inline_lambda_messages.override(parent.inline_lambda_messages);
    generate_return_types.override(parent.generate_return_types);
    generate_condition_statements.override(
        parent.generate_condition_statements);
    debug_mode.override(parent.debug_mode);
    generate_metadata.override(parent.generate_metadata);
    allow_empty_diagrams.override(parent.allow_empty_diagrams);
    type_aliases.override(parent.type_aliases);
    user_data.override(parent.user_data);
}

std::string inheritable_diagram_options::simplify_template_type(
    std::string full_name) const
{
    type_aliases_longer_first_t aliases;
    aliases.insert(type_aliases().begin(), type_aliases().end());

    bool matched{true};
    while (matched) {
        auto matched_in_iteration{false};
        for (const auto &[pattern, replacement] : aliases) {
            matched_in_iteration =
                util::replace_all(full_name, pattern, replacement) ||
                matched_in_iteration;
        }
        matched = matched_in_iteration;
    }

    return full_name;
}

bool inheritable_diagram_options::generate_fully_qualified_name() const
{
    return (package_type() == package_type_t::kNamespace) &&
        !generate_packages();
}

std::vector<std::string> diagram::glob_translation_units(
    const std::vector<std::string> &compilation_database_files) const
{

    // Make sure that the paths are in preferred format for a given platform
    // before intersecting the matches with compliation database
    std::vector<std::string> compilation_database_paths;
    compilation_database_paths.reserve(compilation_database_files.size());
    for (const auto &cdf : compilation_database_files) {
        std::filesystem::path p{cdf};
        p.make_preferred();
        compilation_database_paths.emplace_back(p.string());
    }

    // If glob is not defined use all translation units from the
    // compilation database
    if (!glob.has_value || (glob().include.empty() && glob().exclude.empty())) {
        return compilation_database_paths;
    }

    // Otherwise, get all translation units matching the glob from diagram
    // configuration
    std::vector<std::string> glob_matches{};

    LOG_DBG("Looking for translation units in {}", root_directory().string());

    for (const auto &g : glob().include) {
        if (g.is_regex()) {
            LOG_DBG("Matching inclusive glob regex {}", g.to_string());

            std::regex regex_pattern(
                g.to_string(), std::regex_constants::optimize);

            std::copy_if(compilation_database_paths.begin(),
                compilation_database_paths.end(),
                std::back_inserter(glob_matches),
                [&regex_pattern](const auto &tu) {
                    std::smatch m;

                    return exists(std::filesystem::path{tu}) &&
                        std::regex_search(tu, m, regex_pattern);
                });
        }
        else {
            std::filesystem::path absolute_glob_path{g.to_string()};

#ifdef _MSC_VER
            if (!absolute_glob_path.has_root_name())
#else
            if (!absolute_glob_path.is_absolute())
#endif
                absolute_glob_path = root_directory() / absolute_glob_path;

            LOG_DBG("Searching glob path {}", absolute_glob_path.string());

            auto matches = glob::glob(absolute_glob_path.string(), true, false);

            for (const auto &match : matches) {
                const auto path =
                    std::filesystem::canonical(root_directory() / match);

                glob_matches.emplace_back(path.string());
            }
        }
    }

    if (glob().include.empty())
        glob_matches = compilation_database_paths;

    for (const auto &g : glob().exclude) {
        if (g.is_regex()) {
            LOG_DBG("Matching exclusive glob regex {}", g.to_string());

            std::regex regex_pattern(
                g.to_string(), std::regex_constants::optimize);

            for (const auto &cdf : compilation_database_paths) {
                std::smatch m;
                if (std::regex_search(cdf, m, regex_pattern)) {
                    glob_matches.erase(
                        remove(begin(glob_matches), end(glob_matches), cdf),
                        glob_matches.end());
                }
            }
        }
        else {
            std::filesystem::path absolute_glob_path{g.to_string()};

#ifdef _MSC_VER
            if (!absolute_glob_path.has_root_name())
#else
            if (!absolute_glob_path.is_absolute())
#endif
                absolute_glob_path = root_directory() / absolute_glob_path;

            LOG_DBG("Searching exclusive glob path {}",
                absolute_glob_path.string());

            auto matches = glob::glob(absolute_glob_path.string(), true, false);

            for (const auto &match : matches) {
                const auto path =
                    std::filesystem::canonical(root_directory() / match);

                glob_matches.erase(remove(begin(glob_matches),
                                       end(glob_matches), path.string()),
                    glob_matches.end());
            }
        }
    }

    // Calculate intersection between glob matches and compilation database
    std::vector<std::string> result;
    for (const auto &gm : glob_matches) {
        std::filesystem::path gm_path{gm};
        gm_path.make_preferred();
        if (util::contains(compilation_database_paths, gm_path) ||
            util::contains(compilation_database_files, gm)) {
            result.emplace_back(gm_path.string());
        }
    }

    return result;
}

std::filesystem::path diagram::make_path_relative(
    const std::filesystem::path &p) const
{
    return relative(p, root_directory()).lexically_normal().string();
}

std::vector<std::string> diagram::make_module_relative(
    const std::optional<std::string> &maybe_module) const
{
    if (!maybe_module)
        return {};

    auto module_path = common::model::path(
        maybe_module.value(), common::model::path_type::kModule)
                           .tokens();

    if (using_module.has_value) {
        auto using_module_path = common::model::path(
            using_module(), common::model::path_type::kModule)
                                     .tokens();

        if (util::starts_with(module_path, using_module_path)) {
            util::remove_prefix(module_path, using_module_path);
        }
    }

    return module_path;
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
