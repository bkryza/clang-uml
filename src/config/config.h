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

enum class diagram_type { class_diagram, sequence_diagram, package_diagram };
enum class method_arguments { full, abbreviated, none };

struct plantuml {
    std::vector<std::string> before;
    std::vector<std::string> after;

    void append(const plantuml &r);
};

struct filter {
    std::vector<std::string> namespaces;

    // Valid values are:
    //   - inheritance
    //   - dependency
    //   - instantiation
    std::vector<std::string> relationships;

    // E.g.:
    //   - classes
    //   - enums
    std::vector<std::string> entity_types;

    // E.g.:
    //   - public
    //   - private
    std::vector<common::model::scope_t> scopes;
};

enum class hint_t { up, down, left, right };

struct layout_hint {
    hint_t hint;
    std::string entity;
};

using layout_hints = std::map<std::string, std::vector<layout_hint>>;

std::string to_string(const diagram_type t);
std::string to_string(const hint_t t);

struct inheritable_diagram_options {
    option<std::vector<std::string>> glob{"glob"};
    option<std::vector<std::string>> using_namespace{"using_namespace"};
    option<bool> include_relations_also_as_members{
        "include_relations_also_as_members", true};
    option<filter> include{"include"};
    option<filter> exclude{"exclude"};
    option<plantuml> puml{"plantuml", option_inherit_mode::append};
    option<method_arguments> generate_method_arguments{
        "generate_method_arguments", method_arguments::full};
    option<bool> generate_packages{
        "generate_packages", false};

    void inherit(const inheritable_diagram_options &parent);
};

struct diagram : public inheritable_diagram_options {
    virtual ~diagram() = default;

    virtual diagram_type type() const = 0;

    std::string name;

    bool should_include_entities(const std::string &ent);

    bool should_include_relationship(const std::string &rel);

    bool should_include(const std::string &name_) const;

    bool should_include(const common::model::scope_t scope) const;
};

struct source_location {
    enum class location_t { usr, marker, fileline, function };
    location_t location_type;
    std::string location;
};

struct class_diagram : public diagram {
    virtual ~class_diagram() = default;

    diagram_type type() const override;

    option<std::vector<std::string>> classes{"classes"};
    option<layout_hints> layout{"layout"};

    bool has_class(std::string clazz);
};

struct sequence_diagram : public diagram {
    virtual ~sequence_diagram() = default;

    diagram_type type() const override;

    option<std::vector<source_location>> start_from{"start_from"};
};

struct package_diagram : public diagram {
    virtual ~package_diagram() = default;

    diagram_type type() const override;

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
}
}
