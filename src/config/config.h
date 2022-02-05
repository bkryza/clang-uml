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

struct plantuml {
    std::vector<std::string> before;
    std::vector<std::string> after;

    void append(const plantuml &r)
    {
        before.insert(before.end(), r.before.begin(), r.before.end());
        after.insert(after.end(), r.after.begin(), r.after.end());
    }
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

enum class diagram_type { class_diagram, sequence_diagram, package_diagram };

std::string to_string(const diagram_type t);

template <typename T> void append_value(T &l, const T &r) { l = r; }
// template<> void append_value<bool>(bool &l, const bool&r) {l = r;}

template <typename T> struct option {
    option(const std::string &name_)
        : name{name_}
    {
    }

    option(const std::string &name_, const T &initial_value)
        : name{name_}
        , value{initial_value}
        , has_value{true}
    {
    }

    std::string name;

    T value;
    bool has_value{false};

    void append(const T &r)
    {
        append_value(value, r);
        has_value = true;
    }

    option<T> &operator+=(const T &r)
    {
        append_value(value, r);
        has_value = true;
        return *this;
    }

    T &operator()() { return value; }

    const T &operator()() const { return value; }
};

struct inheritable_diagram_options {
    option<std::vector<std::string>> glob{"glob"};
    option<std::vector<std::string>> using_namespace{"using_namespace"};
    option<bool> include_relations_also_as_members{
        "include_relations_also_as_members", true};
    option<filter> include{"include"};
    option<filter> exclude{"exclude"};
    option<plantuml> puml{"plantuml"};

    void inherit(const inheritable_diagram_options &parent)
    {
        if (!glob.has_value && parent.glob.has_value)
            glob.append(parent.glob());

        if (!using_namespace.has_value && parent.using_namespace.has_value)
            using_namespace.append(parent.using_namespace());

        if (!include_relations_also_as_members.has_value &&
            parent.include_relations_also_as_members.has_value)
            include_relations_also_as_members.append(
                parent.include_relations_also_as_members());

        if (!include.has_value && parent.include.has_value)
            include.append(parent.include());

        if (!exclude.has_value && parent.exclude.has_value)
            exclude.append(parent.exclude());

        if (!puml.has_value && parent.puml.has_value)
            puml.append(parent.puml());
    }
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
