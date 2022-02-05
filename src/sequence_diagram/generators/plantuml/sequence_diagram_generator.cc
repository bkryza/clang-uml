/**
 * src/sequence_diagram/generators/plantuml/sequence_diagram_generator.cc
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

#include "sequence_diagram_generator.h"

#include "sequence_diagram/visitor/translation_unit_context.h"

#include <cppast/libclang_parser.hpp>
#include <cppast/parser.hpp>

namespace clanguml::sequence_diagram::generators::plantuml {

using diagram_model = clanguml::sequence_diagram::model::diagram;
using diagram_config = clanguml::config::sequence_diagram::diagram;
using clanguml::config::source_location;
using clanguml::sequence_diagram::model::activity;
using clanguml::sequence_diagram::model::message;
using clanguml::sequence_diagram::model::message_t;
using clanguml::sequence_diagram::visitor::translation_unit_context;
using namespace clanguml::util;

//
// generator
//

generator::generator(
    clanguml::config::sequence_diagram &config, diagram_model &model)
    : m_config(config)
    , m_model(model)
{
}

std::string generator::to_string(message_t r) const
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

void generator::generate_call(const message &m, std::ostream &ostr) const
{
    const auto from = ns_relative(m_config.using_namespace(), m.from);
    const auto to = ns_relative(m_config.using_namespace(), m.to);

    ostr << '"' << from << "\" "
         << "->"
         << " \"" << to << "\" : " << m.message << "()" << std::endl;
}

void generator::generate_return(const message &m, std::ostream &ostr) const
{
    // Add return activity only for messages between different actors and
    // only if the return type is different than void
    if ((m.from != m.to) && (m.return_type != "void")) {
        const auto from = ns_relative(m_config.using_namespace(), m.from);
        const auto to = ns_relative(m_config.using_namespace(), m.to);

        ostr << '"' << to << "\" "
             << "-->"
             << " \"" << from << "\"" << std::endl;
    }
}

void generator::generate_activity(const activity &a, std::ostream &ostr) const
{
    for (const auto &m : a.messages) {
        const auto to = ns_relative(m_config.using_namespace(), m.to);
        generate_call(m, ostr);
        ostr << "activate " << '"' << to << '"' << std::endl;
        if (m_model.sequences.find(m.to_usr) != m_model.sequences.end())
            generate_activity(m_model.sequences[m.to_usr], ostr);
        generate_return(m, ostr);
        ostr << "deactivate " << '"' << to << '"' << std::endl;
    }
}

void generator::generate(std::ostream &ostr) const
{
    ostr << "@startuml" << std::endl;

    for (const auto &b : m_config.puml().before)
        ostr << b << std::endl;

    for (const auto &sf : m_config.start_from()) {
        if (sf.location_type == source_location::location_t::function) {
            std::uint_least64_t start_from;
            for (const auto &[k, v] : m_model.sequences) {
                if (v.from == sf.location) {
                    start_from = k;
                    break;
                }
            }
            generate_activity(m_model.sequences[start_from], ostr);
        }
        else {
            // TODO: Add support for other sequence start location types
            continue;
        }
    }
    for (const auto &a : m_config.puml().after)
        ostr << a << std::endl;

    ostr << "@enduml" << std::endl;
}

std::ostream &operator<<(std::ostream &os, const generator &g)
{
    g.generate(os);
    return os;
}

clanguml::sequence_diagram::model::diagram generate(
    cppast::libclang_compilation_database &db, const std::string &name,
    clanguml::config::sequence_diagram &diagram)
{
    spdlog::info("Generating diagram {}.puml", name);
    clanguml::sequence_diagram::model::diagram d;
    d.name = name;

    cppast::cpp_entity_index idx;
    cppast::simple_file_parser<cppast::libclang_parser> parser{
        type_safe::ref(idx)};

    clanguml::sequence_diagram::visitor::translation_unit_visitor visitor(
        idx, d, diagram);

    // Get all translation units matching the glob from diagram
    // configuration
    std::vector<std::string> translation_units{};
    for (const auto &g : diagram.glob()) {
        spdlog::debug("Processing glob: {}", g);
        const auto matches = glob::rglob(g);
        std::copy(matches.begin(), matches.end(),
            std::back_inserter(translation_units));
    }
    cppast::parse_files(parser, translation_units, db);

    for (auto &file : parser.files())
        visitor(file);

    return d;
}
}
