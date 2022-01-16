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
#include "class_diagram/model/enums.h"
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
    std::vector<class_diagram::model::scope_t> scopes;
};

struct diagram {
    virtual ~diagram() = default;

    std::string name;
    std::vector<std::string> glob;
    std::vector<std::string> using_namespace;

    filter include;
    filter exclude;

    plantuml puml;

    bool should_include_entities(const std::string &ent);

    bool should_include_relationship(const std::string &rel);

    bool should_include(const std::string &name_) const;

    bool should_include(const class_diagram::model::scope_t scope) const;
};

struct source_location {
    enum class location_t { usr, marker, fileline, function };
    location_t location_type;
    std::string location;
};

struct class_diagram : public diagram {
    virtual ~class_diagram() = default;

    std::vector<std::string> classes;
    bool include_relations_also_as_members{true};

    bool has_class(std::string clazz);
};

struct sequence_diagram : public diagram {
    virtual ~sequence_diagram() = default;

    std::vector<source_location> start_from;
};

struct config {
    // the glob list is additive and relative to the current
    // directory
    std::vector<std::string> glob;
    std::string compilation_database_dir{"."};
    std::string output_directory{};
    std::map<std::string, std::shared_ptr<diagram>> diagrams;
};

config load(const std::string &config_file);
}
}
