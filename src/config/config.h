#pragma once

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

struct diagram {
    virtual ~diagram() = default;

    std::string name;
    std::vector<std::string> glob;
    std::vector<std::string> using_namespace;
    plantuml puml;
};

struct source_location {
    using usr = std::string;
    using marker = std::string;
    using file = std::pair<std::string, int>;
    // std::variant requires unique types, so we cannot add
    // marker here, we need sth like boost::mp_unique
    // type function
    using variant = std::variant<usr, file>;
};

enum class class_scopes { public_, protected_, private_ };

struct class_diagram : public diagram {
    virtual ~class_diagram() = default;

    std::vector<std::string> classes;
    std::vector<class_scopes> methods;
    std::vector<class_scopes> members;

    bool has_class(std::string clazz)
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
};

struct sequence_diagram : public diagram {
    virtual ~sequence_diagram() = default;

    std::vector<source_location::variant> start_from;
};

struct config {
    // the glob list is additive and relative to the current
    // directory
    std::vector<std::string> glob;
    std::string compilation_database_dir{"."};
    std::map<std::string, std::shared_ptr<diagram>> diagrams;
};

config load(const std::string &config_file);
}
}

namespace YAML {
using clanguml::config::class_diagram;
using clanguml::config::config;
using clanguml::config::plantuml;
using clanguml::config::sequence_diagram;
using clanguml::config::source_location;

//
// class_diagram Yaml decoder
//
template <> struct convert<class_diagram> {
    static bool decode(const Node &node, class_diagram &rhs)
    {
        rhs.using_namespace =
            node["using_namespace"].as<std::vector<std::string>>();
        rhs.glob = node["glob"].as<std::vector<std::string>>();
        if (node["puml"])
            rhs.puml = node["plantuml"].as<plantuml>();
        return true;
    }
};

template <> struct convert<std::vector<source_location::variant>> {
    static bool decode(
        const Node &node, std::vector<source_location::variant> &rhs)
    {
        for (auto it = node.begin(); it != node.end(); ++it) {
            const YAML::Node &n = *it;
            if (n["usr"]) {
                rhs.emplace_back(n["usr"].as<source_location::usr>());
            }
            else if (n["marker"]) {
                rhs.emplace_back(n["marker"].as<source_location::marker>());
            }
            else if (n["file"] && n["line"]) {
                rhs.emplace_back(std::make_pair(
                    n["file"].as<std::string>(), n["line"].as<int>()));
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
// sequence_diagram Yaml decoder
//
template <> struct convert<sequence_diagram> {
    static bool decode(const Node &node, sequence_diagram &rhs)
    {
        rhs.using_namespace =
            node["using_namespace"].as<std::vector<std::string>>();
        rhs.glob = node["glob"].as<std::vector<std::string>>();
        rhs.puml = node["plantuml"].as<plantuml>();

        if (node["start_from"])
            rhs.start_from = node["start_from"].as<decltype(rhs.start_from)>();
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

        if (node["compilation_database_dir"])
            rhs.compilation_database_dir =
                node["compilation_database_dir"].as<std::string>();

        auto diagrams = node["diagrams"];

        assert(diagrams.Type() == NodeType::Map);

        for (const auto &d : diagrams) {
            const auto diagram_type = d.second["type"].as<std::string>();
            if (diagram_type == "class") {
                rhs.diagrams[d.first.as<std::string>()] =
                    std::make_shared<class_diagram>(
                        d.second.as<class_diagram>());
            }
            if (diagram_type == "sequence") {
                rhs.diagrams[d.first.as<std::string>()] =
                    std::make_shared<sequence_diagram>(
                        d.second.as<sequence_diagram>());
            }
            else {
                spdlog::warn(
                    "Diagrams of type {} are not supported at the moment... ",
                    diagram_type);
            }
        }

        return true;
    }
};
}

