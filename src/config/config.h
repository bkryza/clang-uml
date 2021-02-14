#pragma once

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace clanguml {
namespace config {

struct diagram {
    virtual ~diagram() = default;

    std::string name;
    std::vector<std::string> glob;
    std::vector<std::string> puml;
    std::string using_namespace;
};

enum class class_scopes { public_, protected_, private_ };

struct class_diagram : public diagram {
    virtual ~class_diagram() = default;

    std::vector<std::string> classes;
    std::vector<class_scopes> methods;
    std::vector<class_scopes> members;

    bool has_class(std::string clazz)
    {
        spdlog::debug("CHECKING IF {} IS WHITE LISTED", clazz);
        for (const auto &c : classes) {
            std::string prefix{};
            if (!using_namespace.empty()) {
                prefix = using_namespace + "::";
            }
            if (prefix + c == clazz)
                return true;
        }

        return false;
    }
};

struct source_location {
    std::string file;
    unsigned int line;
};

struct sequence_diagram : public diagram {
    virtual ~sequence_diagram() = default;

    std::optional<source_location> start_from;
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
using clanguml::config::sequence_diagram;
using clanguml::config::source_location;

//
// class_diagram Yaml decoder
//
template <> struct convert<class_diagram> {
    static bool decode(const Node &node, class_diagram &rhs)
    {
        rhs.using_namespace = node["using_namespace"].as<std::string>();
        rhs.glob = node["glob"].as<std::vector<std::string>>();
        rhs.puml = node["puml"].as<std::vector<std::string>>();
        rhs.classes = node["classes"].as<std::vector<std::string>>();
        return true;
    }
};

template <> struct convert<source_location> {
    static bool decode(const Node &node, source_location &rhs)
    {
        rhs.file = node["file"].as<std::string>();
        rhs.line = node["line"].as<unsigned int>();
        return true;
    }
};


//
// sequence_diagram Yaml decoder
//
template <> struct convert<sequence_diagram> {
    static bool decode(const Node &node, sequence_diagram &rhs)
    {
        rhs.using_namespace = node["using_namespace"].as<std::string>();
        rhs.glob = node["glob"].as<std::vector<std::string>>();
        rhs.puml = node["puml"].as<std::vector<std::string>>();

        if(node["start_from"])
            rhs.start_from = node["start_from"].as<source_location>();
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

