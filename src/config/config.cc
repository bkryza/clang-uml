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
        auto config_file_path =
            std::filesystem::absolute(std::filesystem::path{config_file});
        doc.force_insert(
            "__parent_path", config_file_path.parent_path().string());

        // If the current directory is also a git repository,
        // load some config values, which can be included in the
        // generated diagrams
        if (util::is_git_repository() && !doc["git"]) {
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

    if (!relationship_hints().count("std::vector")) {
        relationship_hints().insert({"std::vector", {}});
    }
    if (!relationship_hints().count("std::unique_ptr")) {
        relationship_hints().insert({"std::unique_ptr", {}});
    }
    if (!relationship_hints().count("std::shared_ptr")) {
        relationship_hints().insert(
            {"std::shared_ptr", {relationship_t::kAssociation}});
    }
    if (!relationship_hints().count("std::weak_ptr")) {
        relationship_hints().insert(
            {"std::weak_ptr", {relationship_t::kAssociation}});
    }
    if (!relationship_hints().count("std::tuple")) {
        relationship_hints().insert({"std::tuple", {}});
    }
    if (!relationship_hints().count("std::map")) {
        relationship_hint_t hint{relationship_t::kNone};
        hint.argument_hints.insert({1, relationship_t::kAggregation});
        relationship_hints().insert({"std::tuple", std::move(hint)});
    }
}

void class_diagram::initialize_template_aliases()
{
    if (!template_aliases().count("std::basic_string<char>")) {
        template_aliases().insert({"std::basic_string<char>", "std::string"});
    }
    if (!template_aliases().count("std::basic_string<char,std::char_traits<"
                                  "char>,std::allocator<char>>")) {
        template_aliases().insert({"std::basic_string<char,std::char_traits<"
                                   "char>,std::allocator<char>>",
            "std::string"});
    }
    if (!template_aliases().count("std::basic_string<wchar_t>")) {
        template_aliases().insert(
            {"std::basic_string<wchar_t>", "std::wstring"});
    }
    if (!template_aliases().count("std::basic_string<char16_t>")) {
        template_aliases().insert(
            {"std::basic_string<char16_t>", "std::u16string"});
    }
    if (!template_aliases().count("std::basic_string<char32_t>")) {
        template_aliases().insert(
            {"std::basic_string<char32_t>", "std::u32string"});
    }
    if (!template_aliases().count("std::integral_constant<bool,true>")) {
        template_aliases().insert(
            {"std::integral_constant<bool,true>", "std::true_type"});
    }
    if (!template_aliases().count("std::integral_constant<bool,false>")) {
        template_aliases().insert(
            {"std::integral_constant<bool,false>", "std::false_type"});
    }
}

template <> void append_value<plantuml>(plantuml &l, const plantuml &r)
{
    l.append(r);
}
}
}

namespace YAML {
using clanguml::common::model::access_t;
using clanguml::common::model::relationship_t;
using clanguml::config::class_diagram;
using clanguml::config::config;
using clanguml::config::filter;
using clanguml::config::generate_links_config;
using clanguml::config::git_config;
using clanguml::config::hint_t;
using clanguml::config::include_diagram;
using clanguml::config::layout_hint;
using clanguml::config::method_arguments;
using clanguml::config::package_diagram;
using clanguml::config::plantuml;
using clanguml::config::relationship_hint_t;
using clanguml::config::sequence_diagram;
using clanguml::config::source_location;

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
            option.set({node[option.name].template as<std::string>()});
        else if (node[option.name].Type() == NodeType::Sequence)
            option.set(
                {node[option.name].template as<std::vector<std::string>>()[0]});
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
    else if (diagram_type == "include") {
        return std::make_shared<include_diagram>(d.as<include_diagram>());
    }

    LOG_ERROR("Diagrams of type {} are not supported... ", diagram_type);

    return {};
}

//
// config std::filesystem::path decoder
//
template <> struct convert<std::filesystem::path> {
    static bool decode(const Node &node, std::filesystem::path &rhs)
    {
        if (!node.IsScalar())
            return false;

        rhs = std::filesystem::path{node.as<std::string>()};

        return true;
    }
};

//
// config access_t decoder
//
template <> struct convert<access_t> {
    static bool decode(const Node &node, access_t &rhs)
    {
        if (node.as<std::string>() == "public")
            rhs = access_t::kPublic;
        else if (node.as<std::string>() == "protected")
            rhs = access_t::kProtected;
        else if (node.as<std::string>() == "private")
            rhs = access_t::kPrivate;
        else
            return false;

        return true;
    }
};

//
// config relationship_t decoder
//
template <> struct convert<relationship_t> {
    static bool decode(const Node &node, relationship_t &rhs)
    {
        assert(node.Type() == NodeType::Scalar);

        auto relationship_name = node.as<std::string>();
        if (relationship_name == "extension" ||
            relationship_name == "inheritance") {
            rhs = relationship_t::kExtension;
        }
        else if (relationship_name == "composition") {
            rhs = relationship_t::kComposition;
        }
        else if (relationship_name == "aggregation") {
            rhs = relationship_t::kAggregation;
        }
        else if (relationship_name == "containment") {
            rhs = relationship_t::kContainment;
        }
        else if (relationship_name == "ownership") {
            rhs = relationship_t::kOwnership;
        }
        else if (relationship_name == "association") {
            rhs = relationship_t::kAssociation;
        }
        else if (relationship_name == "instantiation") {
            rhs = relationship_t::kInstantiation;
        }
        else if (relationship_name == "friendship") {
            rhs = relationship_t::kFriendship;
        }
        else if (relationship_name == "dependency") {
            rhs = relationship_t::kDependency;
        }
        else if (relationship_name == "none") {
            rhs = relationship_t::kNone;
        }
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
                rhs.namespaces.push_back({ns});
        }

        if (node["relationships"])
            rhs.relationships =
                node["relationships"].as<decltype(rhs.relationships)>();

        if (node["elements"])
            rhs.elements = node["elements"].as<decltype(rhs.elements)>();

        if (node["access"])
            rhs.access = node["access"].as<decltype(rhs.access)>();

        if (node["subclasses"])
            rhs.subclasses = node["subclasses"].as<decltype(rhs.subclasses)>();

        if (node["specializations"])
            rhs.specializations =
                node["specializations"].as<decltype(rhs.specializations)>();

        if (node["dependants"])
            rhs.dependants = node["dependants"].as<decltype(rhs.dependants)>();

        if (node["dependencies"])
            rhs.dependencies =
                node["dependencies"].as<decltype(rhs.dependencies)>();

        if (node["context"])
            rhs.context = node["context"].as<decltype(rhs.context)>();

        if (node["paths"])
            rhs.paths = node["paths"].as<decltype(rhs.paths)>();

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
        get_option(node, rhs.relationship_hints);
        get_option(node, rhs.template_aliases);

        rhs.initialize_relationship_hints();
        rhs.initialize_template_aliases();

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
// package_diagram Yaml decoder
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
// include_diagram Yaml decoder
//
template <> struct convert<include_diagram> {
    static bool decode(const Node &node, include_diagram &rhs)
    {
        if (!decode_diagram(node, rhs))
            return false;

        get_option(node, rhs.layout);
        get_option(node, rhs.relative_to);
        get_option(node, rhs.generate_system_headers);

        // Convert the path in relative_to to an absolute path, with respect
        // to the directory where the `.clang-uml` configuration file is located
        if (rhs.relative_to) {
            auto absolute_relative_to =
                std::filesystem::path{node["__parent_path"].as<std::string>()} /
                rhs.relative_to();
            rhs.relative_to.set(absolute_relative_to.lexically_normal());
        }

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
// relationship_hint_t Yaml decoder
//
template <> struct convert<relationship_hint_t> {
    static bool decode(const Node &node, relationship_hint_t &rhs)
    {
        assert(node.Type() == NodeType::Map || node.Type() == NodeType::Scalar);

        if (node.Type() == NodeType::Scalar) {
            // This will be default relationship hint for all arguments
            // of this template (useful for instance for tuples)
            rhs.default_hint = node.as<relationship_t>();
        }
        else {
            for (const auto &it : node) {
                auto key = it.first.as<std::string>();
                if (key == "default") {
                    rhs.default_hint = node["default"].as<relationship_t>();
                }
                else {
                    try {
                        auto index = stoul(key);
                        rhs.argument_hints[index] =
                            it.second.as<relationship_t>();
                    }
                    catch (std::exception &e) {
                        return false;
                    }
                }
            }
        }

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
        get_option(node, rhs.generate_system_headers);
        get_option(node, rhs.git);
        rhs.base_directory.set(node["__parent_path"].as<std::string>());
        get_option(node, rhs.relative_to);

        auto diagrams = node["diagrams"];

        assert(diagrams.Type() == NodeType::Map);

        for (auto d : diagrams) {
            auto name = d.first.as<std::string>();
            std::shared_ptr<clanguml::config::diagram> diagram_config{};
            auto parent_path = node["__parent_path"].as<std::string>();

            if (has_key(d.second, "include!")) {
                auto include_path = std::filesystem::path{parent_path};
                include_path /= d.second["include!"].as<std::string>();

                YAML::Node node = YAML::LoadFile(include_path.string());
                node.force_insert("__parent_path", parent_path);

                diagram_config = parse_diagram_config(node);
            }
            else {
                d.second.force_insert("__parent_path", parent_path);
                diagram_config = parse_diagram_config(d.second);
            }

            if (diagram_config) {
                diagram_config->name = name;
                diagram_config->inherit(rhs);
                rhs.diagrams[name] = diagram_config;
            }
            else {
                return false;
            }
        }

        return true;
    }
};
}
