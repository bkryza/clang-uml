/**
 * src/puml/sequence_diagram_generator.h
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
#include "uml/sequence_diagram_model.h"
#include "uml/sequence_diagram_visitor.h"
#include "util/util.h"

#include <glob/glob.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace generators {
namespace sequence_diagram {
namespace puml {

using diagram_model = clanguml::model::sequence_diagram::diagram;
using diagram_config = clanguml::config::sequence_diagram::diagram;
using clanguml::config::source_location;
using clanguml::model::sequence_diagram::activity;
using clanguml::model::sequence_diagram::message;
using clanguml::model::sequence_diagram::message_t;
using clanguml::visitor::sequence_diagram::tu_context;
using namespace clanguml::util;

class generator {
public:
    generator(clanguml::config::sequence_diagram &config, diagram_model &model)
        : m_config(config)
        , m_model(model)
    {
    }

    std::string to_string(message_t r) const
    {
        switch (r) {
            case message_t::kCall:
                return "->";
            case message_t::kReturn:
                return "<--";
            default:
                return "";
        }
    }

    void generate_call(const message &m, std::ostream &ostr) const
    {
        const auto from = ns_relative(m_config.using_namespace, m.from);
        const auto to = ns_relative(m_config.using_namespace, m.to);

        ostr << '"' << from << "\" "
             << "->"
             << " \"" << to << "\" : " << m.message << "()" << std::endl;
    }

    void generate_return(const message &m, std::ostream &ostr) const
    {
        if ((m.from != m.to) && (m.return_type != "void")) {
            const auto from = ns_relative(m_config.using_namespace, m.from);
            const auto to = ns_relative(m_config.using_namespace, m.to);

            ostr << '"' << to << "\" "
                 << "-->"
                 << " \"" << from << "\"" << std::endl;
        }
    }

    void generate_activity(const activity &a, std::ostream &ostr) const
    {
        for (const auto &m : a.messages) {
            const auto to = ns_relative(m_config.using_namespace, m.to);
            generate_call(m, ostr);
            ostr << "activate " << '"' << to << '"' << std::endl;
            if (m_model.sequences.find(m.to_usr) != m_model.sequences.end())
                generate_activity(m_model.sequences[m.to_usr], ostr);
            generate_return(m, ostr);
            ostr << "deactivate " << '"' << to << '"' << std::endl;
        }
    }

    void generate(std::ostream &ostr) const
    {
        ostr << "@startuml" << std::endl;

        for (const auto &b : m_config.puml.before)
            ostr << b << std::endl;

        for (const auto &sf : m_config.start_from) {
            std::string start_from;
            if (std::holds_alternative<source_location::usr>(sf)) {
                start_from = std::get<source_location::usr>(sf);
            }
            else {
                // TODO: Add support for other sequence start location types
                continue;
            }
            generate_activity(m_model.sequences[start_from], ostr);
        }
        for (const auto &a : m_config.puml.after)
            ostr << a << std::endl;

        ostr << "@enduml" << std::endl;
    }

    friend std::ostream &operator<<(std::ostream &os, const generator &g);

private:
    clanguml::config::sequence_diagram &m_config;
    clanguml::model::sequence_diagram::diagram &m_model;
};

std::ostream &operator<<(std::ostream &os, const generator &g)
{
    g.generate(os);
    return os;
}
}

clanguml::model::sequence_diagram::diagram generate(
    clanguml::cx::compilation_database &db, const std::string &name,
    clanguml::config::sequence_diagram &diagram)
{
    spdlog::info("Generating diagram {}.puml", name);
    clanguml::model::sequence_diagram::diagram d;
    d.name = name;

    // Get all translation units matching the glob from diagram
    // configuration
    std::vector<std::filesystem::path> translation_units{};
    for (const auto &g : diagram.glob) {
        spdlog::debug("Processing glob: {}", g);
        const auto matches = glob::glob(g);
        std::copy(matches.begin(), matches.end(),
            std::back_inserter(translation_units));
    }

    // Process all matching translation units
    for (const auto &tu_path : translation_units) {
        spdlog::debug("Processing translation unit: {}",
            std::filesystem::canonical(tu_path).c_str());

        auto tu = db.parse_translation_unit(tu_path);

        auto cursor = clang_getTranslationUnitCursor(tu);

        if (clang_Cursor_isNull(cursor)) {
            spdlog::debug("Cursor is NULL");
        }

        spdlog::debug("Cursor kind: {}",
            clang_getCString(clang_getCursorKindSpelling(cursor.kind)));
        spdlog::debug("Cursor name: {}",
            clang_getCString(clang_getCursorDisplayName(cursor)));

        clanguml::visitor::sequence_diagram::tu_context ctx(d, diagram);
        auto res = clang_visitChildren(cursor,
            clanguml::visitor::sequence_diagram::translation_unit_visitor,
            &ctx);

        spdlog::debug("Processing result: {}", res);

        clang_suspendTranslationUnit(tu);
    }

    return d;
}
}
}
}
