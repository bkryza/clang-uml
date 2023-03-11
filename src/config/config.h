/**
 * src/config/config.h
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

#include "class_diagram/model/diagram.h"
#include "common/model/enums.h"
#include "option.h"
#include "util/util.h"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace clanguml {
namespace config {

enum class method_arguments { full, abbreviated, none };

std::string to_string(method_arguments ma);

enum class comment_parser_t { plain, clang };

std::string to_string(comment_parser_t cp);

struct plantuml {
    std::vector<std::string> before;
    std::vector<std::string> after;

    void append(const plantuml &r);
};

struct diagram_template {
    std::string description;
    common::model::diagram_t type;
    std::string jinja_template;
};

struct filter {
    std::vector<common::model::namespace_> namespaces;

    std::vector<std::string> elements;

    // E.g.:
    //   - inheritance/extension
    //   - dependency
    //   - instantiation
    std::vector<common::model::relationship_t> relationships;

    // E.g.:
    //   - public
    //   - protected
    //   - private
    std::vector<common::model::access_t> access;

    std::vector<std::string> subclasses;

    std::vector<std::string> parents;

    std::vector<std::string> specializations;

    std::vector<std::string> dependants;

    std::vector<std::string> dependencies;

    std::vector<std::string> context;

    std::vector<std::filesystem::path> paths;
};

enum class hint_t { up, down, left, right, together, row, column };

std::string to_string(hint_t t);

struct layout_hint {
    hint_t hint{hint_t::up};
    std::variant<std::string, std::vector<std::string>> entity;
};

using layout_hints = std::map<std::string, std::vector<layout_hint>>;

struct generate_links_config {
    std::string link;
    std::string tooltip;
};

struct git_config {
    std::string branch;
    std::string revision;
    std::string commit;
    std::string toplevel;
};

struct relationship_hint_t {
    std::map<unsigned int, common::model::relationship_t> argument_hints;
    common::model::relationship_t default_hint;

    relationship_hint_t(common::model::relationship_t def =
                            common::model::relationship_t::kAggregation)
        : default_hint{def}
    {
    }

    common::model::relationship_t get(unsigned int argument_index) const
    {
        if (argument_hints.count(argument_index) > 0)
            return argument_hints.at(argument_index);

        return default_hint;
    }
};

using relationship_hints_t = std::map<std::string, relationship_hint_t>;

using type_aliases_t = std::map<std::string, std::string>;

enum class location_t { marker, fileline, function };

std::string to_string(location_t cp);

struct source_location {
    location_t location_type{location_t::function};
    std::string location;
};

struct inheritable_diagram_options {
    option<std::vector<std::string>> glob{"glob"};
    option<common::model::namespace_> using_namespace{"using_namespace"};
    option<bool> include_relations_also_as_members{
        "include_relations_also_as_members", true};
    option<filter> include{"include"};
    option<filter> exclude{"exclude"};
    option<plantuml> puml{"plantuml", option_inherit_mode::kAppend};
    option<method_arguments> generate_method_arguments{
        "generate_method_arguments", method_arguments::full};
    option<bool> generate_packages{"generate_packages", false};
    option<generate_links_config> generate_links{"generate_links"};
    option<git_config> git{"git"};
    option<layout_hints> layout{"layout"};
    option<std::filesystem::path> base_directory{"__parent_path"};
    option<std::filesystem::path> relative_to{"relative_to"};
    option<bool> generate_system_headers{"generate_system_headers", false};
    option<relationship_hints_t> relationship_hints{"relationship_hints"};
    option<type_aliases_t> type_aliases{"type_aliases"};
    option<comment_parser_t> comment_parser{
        "comment_parser", comment_parser_t::plain};
    option<bool> combine_free_functions_into_file_participants{
        "combine_free_functions_into_file_participants", false};
    option<std::vector<std::string>> participants_order{"participants_order"};
    option<bool> debug_mode{"debug_mode", false};

    void inherit(const inheritable_diagram_options &parent);

    std::string simplify_template_type(std::string full_name) const;
};

struct diagram : public inheritable_diagram_options {
    virtual ~diagram() = default;

    virtual common::model::diagram_t type() const = 0;

    std::vector<std::string> get_translation_units() const;

    std::optional<std::string> get_together_group(
        const std::string &full_name) const;

    void initialize_type_aliases();

    std::string name;
};

struct class_diagram : public diagram {
    ~class_diagram() override = default;

    common::model::diagram_t type() const override;

    void initialize_relationship_hints();
};

struct sequence_diagram : public diagram {
    ~sequence_diagram() override = default;

    common::model::diagram_t type() const override;

    option<std::vector<source_location>> start_from{"start_from"};
};

struct package_diagram : public diagram {
    ~package_diagram() override = default;

    common::model::diagram_t type() const override;
};

struct include_diagram : public diagram {
    ~include_diagram() override = default;

    common::model::diagram_t type() const override;
};

struct config : public inheritable_diagram_options {
    // the glob list is additive and relative to the current
    // directory
    option<std::string> compilation_database_dir{
        "compilation_database_dir", "."};
    option<std::string> output_directory{"output_directory"};

    option<std::map<std::string, diagram_template>> diagram_templates{
        "diagram_templates"};

    std::map<std::string, std::shared_ptr<diagram>> diagrams;

    void initialize_diagram_templates();
};

//
// YAML serialization emitters
//
YAML::Emitter &operator<<(YAML::Emitter &out, const config &c);

YAML::Emitter &operator<<(
    YAML::Emitter &out, const inheritable_diagram_options &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const filter &f);

YAML::Emitter &operator<<(YAML::Emitter &out, const plantuml &p);

YAML::Emitter &operator<<(YAML::Emitter &out, const method_arguments &ma);

YAML::Emitter &operator<<(YAML::Emitter &out, const generate_links_config &glc);

YAML::Emitter &operator<<(YAML::Emitter &out, const git_config &gc);

YAML::Emitter &operator<<(YAML::Emitter &out, const relationship_hint_t &rh);

YAML::Emitter &operator<<(YAML::Emitter &out, const comment_parser_t &cp);

YAML::Emitter &operator<<(YAML::Emitter &out, const hint_t &h);

YAML::Emitter &operator<<(YAML::Emitter &out, const class_diagram &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const sequence_diagram &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const include_diagram &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const package_diagram &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const layout_hint &c);

#ifdef _MSC_VER
YAML::Emitter &operator<<(YAML::Emitter &out, const std::filesystem::path &p);

YAML::Emitter &operator<<(
    YAML::Emitter &out, const std::vector<std::filesystem::path> &p);
#endif

YAML::Emitter &operator<<(YAML::Emitter &out, const source_location &sc);

template <typename T>
YAML::Emitter &operator<<(YAML::Emitter &out, const option<T> &o)
{
    if (o.has_value) {
        out << YAML::Key << o.name;
        if constexpr (std::is_same_v<T, std::filesystem::path>)
            out << YAML::Value << o.value.string();
        else
            out << YAML::Value << o.value;
    }
    return out;
}

config load(const std::string &config_file,
    std::optional<bool> paths_relative_to_pwd = {});

config load_plain(const std::string &config_file);
} // namespace config

namespace common::model {
YAML::Emitter &operator<<(YAML::Emitter &out, const namespace_ &n);
YAML::Emitter &operator<<(YAML::Emitter &out, const relationship_t &r);
YAML::Emitter &operator<<(YAML::Emitter &out, const access_t &r);
YAML::Emitter &operator<<(YAML::Emitter &out, const diagram_t &d);
} // namespace common::model

} // namespace clanguml
