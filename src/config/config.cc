/**
 * src/config/config.cc
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
#include "config.h"

namespace clanguml {
namespace config {

config load(const std::string &config_file)
{
    YAML::Node doc = YAML::LoadFile(config_file);

    return doc.as<config>();
}

bool diagram::should_include_entities(const std::string &ent)
{
    for (const auto &ex : exclude.entity_types) {
        if (ent == ex)
            return false;
    }

    if (include.entity_types.empty())
        return true;

    for (const auto &in : include.entity_types) {
        if (ent == in)
            return true;
    }

    return false;
}

bool diagram::should_include_relationship(const std::string &rel)
{
    for (const auto &ex : exclude.relationships) {
        if (rel == ex)
            return false;
    }

    if (include.relationships.empty())
        return true;

    for (const auto &in : include.relationships) {
        if (rel == in)
            return true;
    }

    return false;
}

bool diagram::should_include(const std::string &name_) const
{
    auto name = clanguml::util::unqualify(name_);

    for (const auto &ex : exclude.namespaces) {
        if (name.find(ex) == 0) {
            spdlog::debug("Skipping from diagram: {}", name);
            return false;
        }
    }

    // If no inclusive namespaces are provided,
    // allow all
    if (include.namespaces.empty())
        return true;

    for (const auto &in : include.namespaces) {
        if (name.find(in) == 0)
            return true;
    }

    spdlog::debug("Skipping from diagram: {}", name);

    return false;
}

bool diagram::should_include(
    const clanguml::common::model::scope_t scope) const
{
    for (const auto &s : exclude.scopes) {
        if (s == scope)
            return false;
    }

    if (include.scopes.empty())
        return true;

    for (const auto &s : include.scopes) {
        if (s == scope)
            return true;
    }

    return false;
}

bool class_diagram::has_class(std::string clazz)
{
    for (const auto &c : classes) {
        for (const auto &ns : using_namespace) {
            std::string prefix{};
            if (!ns.empty()) {
                prefix = ns + "::";
            }
            if (prefix + c == clazz)
                return true;
        }
    }

    return false;
}

}
}

namespace YAML {
using clanguml::common::model::scope_t;
using clanguml::config::class_diagram;
using clanguml::config::config;
using clanguml::config::filter;
using clanguml::config::plantuml;
using clanguml::config::sequence_diagram;
using clanguml::config::source_location;
template <> struct convert<scope_t> {
    static bool decode(const Node &node, scope_t &rhs)
    {
        if (node.as<std::string>() == "public")
            rhs = scope_t::kPublic;
        else if (node.as<std::string>() == "protected")
            rhs = scope_t::kProtected;
        else if (node.as<std::string>() == "private")
            rhs = scope_t::kPrivate;
        else
            return false;

        return true;
    }
};

template <> struct convert<std::vector<source_location>> {
    static bool decode(const Node &node, std::vector<source_location> &rhs)
    {
        for (auto it = node.begin(); it != node.end(); ++it) {
            const YAML::Node &n = *it;
            if (n["usr"]) {
                source_location loc;
                loc.location_type = source_location::location_t::usr;
                loc.location = n["usr"].as<std::string>();
                rhs.emplace_back(std::move(loc));
            }
            else if (n["marker"]) {
                source_location loc;
                loc.location_type = source_location::location_t::marker;
                loc.location = n["marker"].as<std::string>();
                rhs.emplace_back(std::move(loc));
            }
            else if (n["file"] && n["line"]) {
                source_location loc;
                loc.location_type = source_location::location_t::fileline;
                loc.location = n["file"].as<std::string>() + ":" +
                    n["line"].as<std::string>();
                rhs.emplace_back(std::move(loc));
            }
            else if (n["function"]) {
                source_location loc;
                loc.location_type = source_location::location_t::function;
                loc.location = n["function"].as<std::string>();
                rhs.emplace_back(std::move(loc));
            }
            else {
                return false;
            }
        }

        return true;
    }
};

template <> struct convert<plantuml> {
    static bool decode(const Node &node, plantuml &rhs)
    {
        if (node["before"])
            rhs.before = node["before"].as<decltype(rhs.before)>();

        if (node["after"])
            rhs.after = node["after"].as<decltype(rhs.after)>();
        return true;
    }
};

//
// filter Yaml decoder
//
template <> struct convert<filter> {
    static bool decode(const Node &node, filter &rhs)
    {
        if (node["namespaces"])
            rhs.namespaces = node["namespaces"].as<decltype(rhs.namespaces)>();

        if (node["relationships"])
            rhs.relationships =
                node["relationships"].as<decltype(rhs.relationships)>();

        if (node["entity_types"])
            rhs.entity_types =
                node["entity_types"].as<decltype(rhs.entity_types)>();

        if (node["scopes"])
            rhs.scopes = node["scopes"].as<decltype(rhs.scopes)>();

        return true;
    }
};

template <typename T> bool decode_diagram(const Node &node, T &rhs)
{
    if (node["using_namespace"])
        rhs.using_namespace =
            node["using_namespace"].as<std::vector<std::string>>();

    if (node["glob"])
        rhs.glob = node["glob"].as<std::vector<std::string>>();

    if (node["include"])
        rhs.include = node["include"].as<decltype(rhs.include)>();

    if (node["exclude"])
        rhs.exclude = node["exclude"].as<decltype(rhs.exclude)>();

    if (node["plantuml"])
        rhs.puml = node["plantuml"].as<plantuml>();

    return true;
}

//
// class_diagram Yaml decoder
//
template <> struct convert<class_diagram> {
    static bool decode(const Node &node, class_diagram &rhs)
    {
        if (!decode_diagram(node, rhs))
            return false;

        if (node["include_relations_also_as_members"])
            rhs.include_relations_also_as_members =
                node["include_relations_also_as_members"]
                    .as<decltype(rhs.include_relations_also_as_members)>();

        return true;
    }
};

//
// sequence_diagram Yaml decoder
//
template <> struct convert<sequence_diagram> {
    static bool decode(const Node &node, sequence_diagram &rhs)
    {
        if (!decode_diagram(node, rhs))
            return false;

        if (node["start_from"])
            rhs.start_from =
                node["start_from"].as<std::vector<source_location>>();

        return true;
    }
};

//
// config Yaml decoder
//
template <> struct convert<config> {
    static bool decode(const Node &node, config &rhs)
    {
        if (node["glob"])
            rhs.glob = node["glob"].as<std::vector<std::string>>();

        if (node["output_directory"])
            rhs.output_directory = node["output_directory"].as<std::string>();

        if (node["compilation_database_dir"])
            rhs.compilation_database_dir =
                node["compilation_database_dir"].as<std::string>();

        auto diagrams = node["diagrams"];

        assert(diagrams.Type() == NodeType::Map);

        for (const auto &d : diagrams) {
            const auto diagram_type = d.second["type"].as<std::string>();
            auto name = d.first.as<std::string>();
            if (diagram_type == "class") {
                rhs.diagrams[name] = std::make_shared<class_diagram>(
                    d.second.as<class_diagram>());
            }
            else if (diagram_type == "sequence") {
                rhs.diagrams[name] = std::make_shared<sequence_diagram>(
                    d.second.as<sequence_diagram>());
            }
            else {
                spdlog::warn(
                    "Diagrams of type {} are not supported at the moment... ",
                    diagram_type);
            }
            rhs.diagrams[name]->name = name;
        }

        return true;
    }
};
}
