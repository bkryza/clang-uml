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
#include <filesystem>

namespace clanguml {
namespace config {

config load(const std::string &config_file)
{
    try {
        YAML::Node doc = YAML::LoadFile(config_file);

        // Store the parent path of the config_file to properly resolve
        // the include files paths
        auto config_file_path = std::filesystem::path{config_file};
        doc.force_insert(
            "__parent_path", config_file_path.parent_path().string());

        // If the current directory is also a git repository,
        // load some config values which can be included in the
        // generated diagrams
        if(util::is_git_repository() && !doc["git"]) {
            YAML::Node git_config{YAML::NodeType::Map};
            git_config["branch"] = util::get_git_branch();
            git_config["revision"] = util::get_git_revision();
            git_config["commit"] = util::get_git_commit();
            git_config["toplevel"] = util::get_git_toplevel_dir();

            doc["git"] = git_config;
        }

        return doc.as<config>();
    }
    catch (YAML::BadFile &e) {
        throw std::runtime_error(fmt::format(
            "Could not open config file {}: {}", config_file, e.what()));
    }
    catch (YAML::Exception &e) {
        throw std::runtime_error(fmt::format(
            "Cannot parse YAML file {}: {}", config_file, e.what()));
    }
}

std::string to_string(const diagram_type t)
{
    switch (t) {
    case diagram_type::class_diagram:
        return "class";
    case diagram_type::sequence_diagram:
        return "sequence";
    case diagram_type::package_diagram:
        return "package";
    default:
        assert(false);
    }
}

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
    git.override(parent.git);
}

bool diagram::should_include_entities(const std::string &ent)
{
    for (const auto &ex : exclude().entity_types) {
        if (ent == ex)
            return false;
    }

    if (include().entity_types.empty())
        return true;

    for (const auto &in : include().entity_types) {
        if (ent == in)
            return true;
    }

    return false;
}

bool diagram::should_include_relationship(const std::string &rel)
{
    for (const auto &ex : exclude().relationships) {
        if (rel == ex)
            return false;
    }

    if (include().relationships.empty())
        return true;

    for (const auto &in : include().relationships) {
        if (rel == in)
            return true;
    }

    return false;
}

bool diagram::should_include(
    const std::pair<common::model::namespace_, std::string> &name) const
{
    return should_include(std::get<0>(name), std::get<1>(name));
}

bool diagram::should_include(
    const common::model::namespace_ &ns, const std::string &name) const
{
    return should_include(ns | name);
}

bool diagram::should_include(const std::string &name_) const
{
    auto name = clanguml::util::unqualify(name_);

    for (const auto &ex : exclude().namespaces) {
        if (name.find(ex.to_string()) == 0) {
            LOG_DBG("Skipping from diagram: {}", name);
            return false;
        }
    }

    // If no inclusive namespaces are provided,
    // allow all
    if (include().namespaces.empty())
        return true;

    for (const auto &in : include().namespaces) {
        if (name.find(in.to_string()) == 0)
            return true;
    }

    LOG_DBG("Skipping from diagram: {}", name);

    return false;
}

bool diagram::should_include(const common::model::namespace_ &path) const
{
    return should_include(path.to_string());
}

bool diagram::should_include_package(
    const common::model::namespace_ &path) const
{
    return should_include_package(path.to_string());
}

bool diagram::should_include_package(const std::string &name) const
{

    for (const auto &ex : exclude().namespaces) {
        if (name.find(ex.to_string()) == 0) {
            LOG_DBG("Skipping from diagram: {}", name);
            return false;
        }
    }

    // If no inclusive namespaces are provided,
    // allow all
    if (include().namespaces.empty())
        return true;

    for (const auto &in : include().namespaces) {
        if (in.to_string().find(name) == 0 || name.find(in.to_string()) == 0)
            return true;
    }

    LOG_DBG("Skipping from diagram: {}", name);

    return false;
}

bool diagram::should_include(const clanguml::common::model::scope_t scope) const
{
    for (const auto &s : exclude().scopes) {
        if (s == scope)
            return false;
    }

    if (include().scopes.empty())
        return true;

    for (const auto &s : include().scopes) {
        if (s == scope)
            return true;
    }

    return false;
}

diagram_type class_diagram::type() const { return diagram_type::class_diagram; }

bool class_diagram::has_class(std::string clazz)
{
    for (const auto &c : classes()) {
        for (const auto &ns : using_namespace()) {
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

diagram_type sequence_diagram::type() const
{
    return diagram_type::sequence_diagram;
}

diagram_type package_diagram::type() const
{
    return diagram_type::package_diagram;
}

template <>
void append_value<std::vector<std::string>>(
    std::vector<std::string> &l, const std::vector<std::string> &r)
{
    l.insert(l.end(), r.begin(), r.end());
}

template <> void append_value<plantuml>(plantuml &l, const plantuml &r)
{
    l.append(r);
}
}
}

namespace YAML {
using clanguml::common::model::scope_t;
using clanguml::config::class_diagram;
using clanguml::config::config;
using clanguml::config::filter;
using clanguml::config::generate_links_config;
using clanguml::config::hint_t;
using clanguml::config::layout_hint;
using clanguml::config::method_arguments;
using clanguml::config::package_diagram;
using clanguml::config::plantuml;
using clanguml::config::sequence_diagram;
using clanguml::config::source_location;
using clanguml::config::git_config;

inline bool has_key(const YAML::Node &n, const std::string &key)
{
    assert(n.Type() == NodeType::Map);

    return std::count_if(n.begin(), n.end(), [&key](auto &&n) {
        return n.first.template as<std::string>() == key;
    }) > 0;
}

template <typename T>
void get_option(const Node &node, clanguml::config::option<T> &option)
{
    if (node[option.name])
        option.set(node[option.name].template as<T>());
}

template <>
void get_option<clanguml::common::model::namespace_>(const Node &node,
    clanguml::config::option<clanguml::common::model::namespace_> &option)
{
    if (node[option.name]) {
        if (node[option.name].Type() == NodeType::Scalar)
            option.set(node[option.name].template as<std::string>());
        else if (node[option.name].Type() == NodeType::Sequence)
            option.set(
                node[option.name].template as<std::vector<std::string>>()[0]);
        else
            throw std::runtime_error("Invalid using_namespace value");
    }
}

template <>
void get_option<method_arguments>(
    const Node &node, clanguml::config::option<method_arguments> &option)
{
    if (node[option.name]) {
        const auto &val = node[option.name].as<std::string>();
        if (val == "full")
            option.set(method_arguments::full);
        else if (val == "abbreviated")
            option.set(method_arguments::abbreviated);
        else if (val == "none")
            option.set(method_arguments::none);
        else
            throw std::runtime_error(
                "Invalid generate_method_arguments value: " + val);
    }
}

std::shared_ptr<clanguml::config::diagram> parse_diagram_config(const Node &d)
{
    const auto diagram_type = d["type"].as<std::string>();

    if (diagram_type == "class") {
        return std::make_shared<class_diagram>(d.as<class_diagram>());
    }
    else if (diagram_type == "sequence") {
        return std::make_shared<sequence_diagram>(d.as<sequence_diagram>());
    }
    else if (diagram_type == "package") {
        return std::make_shared<package_diagram>(d.as<package_diagram>());
    }

    LOG_ERROR("Diagrams of type {} are not supported... ", diagram_type);

    return {};
}

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
        if (node["namespaces"]) {
            auto namespace_list =
                node["namespaces"].as<std::vector<std::string>>();
            for (const auto &ns : namespace_list)
                rhs.namespaces.push_back(ns);
        }

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

//
// generate_links_config Yaml decoder
//
template <> struct convert<generate_links_config> {
    static bool decode(const Node &node, generate_links_config &rhs)
    {
        if (node["link"])
            rhs.link = node["link"].as<decltype(rhs.link)>();

        if (node["tooltip"])
            rhs.tooltip = node["tooltip"].as<decltype(rhs.tooltip)>();

        return true;
    }
};

//
// git_config Yaml decoder
//
template <> struct convert<git_config> {
    static bool decode(const Node &node, git_config &rhs)
    {
        if (node["branch"])
            rhs.branch = node["branch"].as<decltype(rhs.branch)>();

        if (node["revision"])
            rhs.revision = node["revision"].as<decltype(rhs.revision)>();

        if (node["commit"])
            rhs.commit = node["commit"].as<decltype(rhs.commit)>();

        if (node["toplevel"])
            rhs.toplevel = node["toplevel"].as<decltype(rhs.toplevel)>();

        return true;
    }
};

template <typename T> bool decode_diagram(const Node &node, T &rhs)
{
    get_option(node, rhs.glob);
    get_option(node, rhs.using_namespace);
    get_option(node, rhs.include);
    get_option(node, rhs.exclude);
    get_option(node, rhs.puml);
    get_option(node, rhs.git);
    get_option(node, rhs.generate_links);

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

        get_option(node, rhs.classes);
        get_option(node, rhs.layout);
        get_option(node, rhs.include_relations_also_as_members);
        get_option(node, rhs.generate_method_arguments);
        get_option(node, rhs.generate_packages);

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

        get_option(node, rhs.start_from);

        return true;
    }
};

//
// class_diagram Yaml decoder
//
template <> struct convert<package_diagram> {
    static bool decode(const Node &node, package_diagram &rhs)
    {
        if (!decode_diagram(node, rhs))
            return false;

        get_option(node, rhs.layout);

        return true;
    }
};

//
// layout_hint Yaml decoder
//
template <> struct convert<layout_hint> {
    static bool decode(const Node &node, layout_hint &rhs)
    {
        assert(node.Type() == NodeType::Map);

        if (node["up"]) {
            rhs.hint = hint_t::up;
            rhs.entity = node["up"].as<std::string>();
        }
        else if (node["down"]) {
            rhs.hint = hint_t::down;
            rhs.entity = node["down"].as<std::string>();
        }
        else if (node["left"]) {
            rhs.hint = hint_t::left;
            rhs.entity = node["left"].as<std::string>();
        }
        else if (node["right"]) {
            rhs.hint = hint_t::right;
            rhs.entity = node["right"].as<std::string>();
        }
        else
            return false;

        return true;
    }
};

//
// config Yaml decoder
//
template <> struct convert<config> {

    static bool decode(const Node &node, config &rhs)
    {
        get_option(node, rhs.glob);
        get_option(node, rhs.using_namespace);
        get_option(node, rhs.output_directory);
        get_option(node, rhs.compilation_database_dir);
        get_option(node, rhs.include_relations_also_as_members);
        get_option(node, rhs.puml);
        get_option(node, rhs.generate_method_arguments);
        get_option(node, rhs.generate_packages);
        get_option(node, rhs.generate_links);
        get_option(node, rhs.git);

        auto diagrams = node["diagrams"];

        assert(diagrams.Type() == NodeType::Map);

        for (const auto &d : diagrams) {
            auto name = d.first.as<std::string>();
            std::shared_ptr<clanguml::config::diagram> diagram_config{};

            if (has_key(d.second, "include!")) {
                auto parent_path = node["__parent_path"].as<std::string>();
                auto include_path = std::filesystem::path{parent_path};
                include_path /= d.second["include!"].as<std::string>();

                YAML::Node node = YAML::LoadFile(include_path.string());

                diagram_config = parse_diagram_config(node);
            }
            else {
                diagram_config = parse_diagram_config(d.second);
            }

            if (diagram_config) {
                diagram_config->name = name;
                diagram_config->inherit(rhs);
                rhs.diagrams[name] = diagram_config;
            }
        }

        return true;
    }
};
}
