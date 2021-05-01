/**
 * src/puml/class_diagram_generator.h
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

#include "config/config.h"
#include "cx/compilation_database.h"
#include "uml/class_diagram_model.h"
#include "uml/class_diagram_visitor.h"
#include "util/util.h"

#include <cppast/cpp_entity_index.hpp>
#include <cppast/libclang_parser.hpp>
#include <glob/glob.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace generators {
namespace class_diagram {
namespace puml {

using diagram_config = clanguml::config::class_diagram::diagram;
using diagram_model = clanguml::model::class_diagram::diagram;
using clanguml::model::class_diagram::class_;
using clanguml::model::class_diagram::class_relationship;
using clanguml::model::class_diagram::element;
using clanguml::model::class_diagram::enum_;
using clanguml::model::class_diagram::relationship_t;
using clanguml::model::class_diagram::scope_t;
using clanguml::visitor::class_diagram::tu_context;
using namespace clanguml::util;

std::string relative_to(std::string n, std::string c)
{
    if (c.rfind(n) == std::string::npos)
        return c;

    return c.substr(n.size() + 2);
}

class generator {
public:
    generator(clanguml::config::class_diagram &config, diagram_model &model)
        : m_config(config)
        , m_model(model)
    {
    }

    std::string to_string(scope_t scope) const
    {
        switch (scope) {
        case scope_t::kPublic:
            return "+";
        case scope_t::kProtected:
            return "#";
        case scope_t::kPrivate:
            return "-";
        default:
            return "";
        }
    }

    std::string to_string(relationship_t r) const
    {
        switch (r) {
        case relationship_t::kOwnership:
        case relationship_t::kComposition:
            return "*--";
        case relationship_t::kAggregation:
            return "o--";
        case relationship_t::kContainment:
            return "--+";
        case relationship_t::kAssociation:
            return "-->";
        case relationship_t::kInstantiation:
            return "..|>";
        case relationship_t::kFriendship:
            return "<..";
        case relationship_t::kDependency:
            return "..>";
        default:
            return "";
        }
    }

    std::string name(relationship_t r) const
    {
        switch (r) {
        case relationship_t::kOwnership:
        case relationship_t::kComposition:
            return "composition";
        case relationship_t::kAggregation:
            return "aggregation";
        case relationship_t::kContainment:
            return "containment";
        case relationship_t::kAssociation:
            return "association";
        case relationship_t::kInstantiation:
            return "instantiation";
        case relationship_t::kFriendship:
            return "friendship";
        case relationship_t::kDependency:
            return "dependency";
        default:
            return "unknown";
        }
    }

    void generate_aliases(const class_ &c, std::ostream &ostr) const
    {
        std::string class_type{"class"};
        if (c.is_abstract())
            class_type = "abstract";

        ostr << class_type << " \"" << c.full_name(m_config.using_namespace);

        ostr << "\" as " << c.alias() << std::endl;
    }

    void generate(const class_ &c, std::ostream &ostr) const
    {
        std::string class_type{"class"};
        if (c.is_abstract())
            class_type = "abstract";

        ostr << class_type << " " << c.alias() << " {" << std::endl;

        //
        // Process methods
        //
        for (const auto &m : c.methods) {
            if (m.is_pure_virtual)
                ostr << "{abstract} ";

            if (m.is_static)
                ostr << "{static} ";

            std::string type{m.type};

            ostr << to_string(m.scope) << m.name;

            ostr << "(";
            if (true) { // TODO: add option to disable parameter generation
                std::vector<std::string> params;
                std::transform(m.parameters.begin(), m.parameters.end(),
                    std::back_inserter(params), [this](const auto &mp) {
                        return mp.to_string(m_config.using_namespace);
                    });
                ostr << fmt::format("{}", fmt::join(params, ", "));
            }
            ostr << ")";

            if (m.is_const)
                ostr << " const";

            assert(!(m.is_pure_virtual && m.is_defaulted));

            if (m.is_pure_virtual)
                ostr << " = 0";

            if (m.is_defaulted)
                ostr << " = default";

            ostr << " : " << ns_relative(m_config.using_namespace, type);

            ostr << std::endl;
        }

        //
        // Process members
        //
        for (const auto &m : c.members) {
            if (m.is_static)
                ostr << "{static} ";

            ostr << to_string(m.scope) << m.name << " : "
                 << ns_relative(m_config.using_namespace, m.type) << std::endl;
        }

        ostr << "}" << std::endl;

        if (m_config.should_include_relationship("inheritance"))
            for (const auto &b : c.bases) {
                ostr << m_model.to_alias(m_config.using_namespace,
                            ns_relative(m_config.using_namespace, b.name))
                     << " <|-- "
                     << m_model.to_alias(m_config.using_namespace,
                            ns_relative(m_config.using_namespace, c.name))
                     << std::endl;
            }

        for (const auto &r : c.relationships) {
            if (!m_config.should_include_relationship(name(r.type)))
                continue;

            std::string destination;
            if (r.destination.find("#") != std::string::npos ||
                r.destination.find("@") != std::string::npos) {
                destination = m_model.usr_to_name(
                    m_config.using_namespace, r.destination);
                if (destination.empty()) {
                    ostr << "' ";
                    destination = r.destination;
                }
            }
            else {
                destination = r.destination;
            }

            ostr << m_model.to_alias(m_config.using_namespace,
                        ns_relative(m_config.using_namespace,
                            c.full_name(m_config.using_namespace)))
                 << " " << to_string(r.type) << " "
                 << m_model.to_alias(m_config.using_namespace,
                        ns_relative(m_config.using_namespace, destination));

            if (!r.label.empty())
                ostr << " : " << r.label;

            ostr << std::endl;
        }
    }

    void generate(const enum_ &e, std::ostream &ostr) const
    {
        ostr << "enum " << ns_relative(m_config.using_namespace, e.name) << " {"
             << std::endl;

        for (const auto &enum_constant : e.constants) {
            ostr << enum_constant << std::endl;
        }

        ostr << "}" << std::endl;

        for (const auto &r : e.relationships) {
            if (!m_config.should_include_relationship(name(r.type)))
                continue;

            std::string destination;
            if (r.destination.find("#") != std::string::npos ||
                r.destination.find("@") != std::string::npos) {
                destination = m_model.usr_to_name(
                    m_config.using_namespace, r.destination);
                if (destination.empty()) {
                    ostr << "' ";
                    destination = r.destination;
                }
            }
            else {
                destination = r.destination;
            }

            ostr << m_model.to_alias(m_config.using_namespace,
                        ns_relative(m_config.using_namespace, e.name))
                 << " " << to_string(r.type) << " "
                 << m_model.to_alias(m_config.using_namespace,
                        ns_relative(m_config.using_namespace, destination));

            if (!r.label.empty())
                ostr << " : " << r.label;

            ostr << std::endl;
        }
    }

    void generate(std::ostream &ostr) const
    {
        ostr << "@startuml" << std::endl;

        for (const auto &b : m_config.puml.before)
            ostr << b << std::endl;

        if (m_config.should_include_entities("classes")) {
            for (const auto &c : m_model.classes) {
                generate_aliases(c, ostr);
                ostr << std::endl;
            }

            for (const auto &c : m_model.classes) {
                generate(c, ostr);
                ostr << std::endl;
            }
        }

        if (m_config.should_include_entities("enums"))
            for (const auto &e : m_model.enums) {
                generate(e, ostr);
                ostr << std::endl;
            }

        for (const auto &b : m_config.puml.after)
            ostr << b << std::endl;

        ostr << "@enduml" << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const generator &g);

private:
    clanguml::config::class_diagram &m_config;
    diagram_model &m_model;
};

std::ostream &operator<<(std::ostream &os, const generator &g)
{
    g.generate(os);
    return os;
}
}

clanguml::model::class_diagram::diagram generate(
    cppast::libclang_compilation_database &db, const std::string &name,
    clanguml::config::class_diagram &diagram)
{
    spdlog::info("Generating diagram {}.puml", name);
    clanguml::model::class_diagram::diagram d;
    d.name = name;

    // Get all translation units matching the glob from diagram
    // configuration
    std::vector<std::string> translation_units{};
    for (const auto &g : diagram.glob) {
        spdlog::debug("Processing glob: {}", g);
        const auto matches = glob::glob(g);
        std::copy(matches.begin(), matches.end(),
            std::back_inserter(translation_units));
    }

    cppast::cpp_entity_index idx;
    cppast::simple_file_parser<cppast::libclang_parser> parser{
        type_safe::ref(idx)};

    // Process all matching translation units
    clanguml::visitor::class_diagram::tu_visitor ctx(idx, d, diagram);
    cppast::parse_files(parser, translation_units, db);
    for (auto &file : parser.files())
        ctx(file);

    return d;
}
}
}
}
