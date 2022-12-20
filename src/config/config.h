/**
 * src/config/config.h
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

enum class comment_parser_t { plain, clang };

struct plantuml {
    std::vector<std::string> before;
    std::vector<std::string> after;

    void append(const plantuml &r);
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

    std::vector<std::string> specializations;

    std::vector<std::string> dependants;

    std::vector<std::string> dependencies;

    std::vector<std::string> context;

    std::vector<std::filesystem::path> paths;
};

enum class hint_t { up, down, left, right };

struct layout_hint {
    hint_t hint{hint_t::up};
    std::string entity;
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

std::string to_string(const hint_t t);

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

    void inherit(const inheritable_diagram_options &parent);

    std::string simplify_template_type(std::string full_name) const;
};

struct diagram : public inheritable_diagram_options {
    virtual ~diagram() = default;

    virtual common::model::diagram_t type() const = 0;

    std::vector<std::string> get_translation_units(
        const std::filesystem::path &root_directory) const;

    void initialize_type_aliases();

    std::string name;
};

struct source_location {
    enum class location_t { usr, marker, fileline, function };
    location_t location_type{location_t::function};
    std::string location;
};

struct class_diagram : public diagram {
    ~class_diagram() override = default;

    common::model::diagram_t type() const override;

    option<std::vector<std::string>> classes{"classes"};
    option<layout_hints> layout{"layout"};

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

    option<layout_hints> layout{"layout"};
};

struct include_diagram : public diagram {
    ~include_diagram() override = default;

    common::model::diagram_t type() const override;

    option<layout_hints> layout{"layout"};
};

struct config : public inheritable_diagram_options {
    // the glob list is additive and relative to the current
    // directory
    option<std::string> compilation_database_dir{
        "compilation_database_dir", "."};
    option<std::string> output_directory{"output_directory"};

    std::map<std::string, std::shared_ptr<diagram>> diagrams;
};

config load(const std::string &config_file);
} // namespace config
} // namespace clanguml
