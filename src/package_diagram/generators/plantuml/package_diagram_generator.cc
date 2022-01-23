/**
 * src/package_diagram/generators/plantuml/package_diagram_generator.cc
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

#include "package_diagram_generator.h"

#include "util/error.h"

namespace clanguml::package_diagram::generators::plantuml {

std::string relative_to(std::string n, std::string c)
{
    if (c.rfind(n) == std::string::npos)
        return c;

    return c.substr(n.size() + 2);
}

generator::generator(
    clanguml::config::package_diagram &config, diagram_model &model)
    : m_config(config)
    , m_model(model)
{
}

std::string generator::to_string(relationship_t r, std::string style) const
{
    switch (r) {
    case relationship_t::kOwnership:
    case relationship_t::kComposition:
        return style.empty() ? "*--" : fmt::format("*-[{}]-", style);
    case relationship_t::kAggregation:
        return style.empty() ? "o--" : fmt::format("o-[{}]-", style);
    case relationship_t::kContainment:
        return style.empty() ? "--+" : fmt::format("-[{}]-+", style);
    case relationship_t::kAssociation:
        return style.empty() ? "-->" : fmt::format("-[{}]->", style);
    case relationship_t::kInstantiation:
        return style.empty() ? "..|>" : fmt::format(".[{}].|>", style);
    case relationship_t::kFriendship:
        return style.empty() ? "<.." : fmt::format("<.[{}].", style);
    case relationship_t::kDependency:
        return style.empty() ? "..>" : fmt::format(".[{}].>", style);
    default:
        return "";
    }
}

std::string generator::name(relationship_t r) const
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
    case relationship_t::kDependency:
        return "dependency";
    default:
        return "unknown";
    }
}

void generator::generate_relationships(
    const package &p, std::ostream &ostr) const
{
    const auto &uns = m_config.using_namespace;

    // Generate this packages relationship
    if (m_config.should_include_relationship("dependency")) {
        LOG_DBG("LOOKING FOR RELATIONSHIPS IN PACKAGE: {}", p.full_name(false));
        for (const auto &r : p.relationships()) {
            std::stringstream relstr;
            try {
                relstr << m_model.to_alias(ns_relative(uns, r.destination()))
                       << " <.. "
                       << m_model.to_alias(ns_relative(uns, p.full_name(false)))
                       << '\n';
                ostr << relstr.str();
            }
            catch (error::uml_alias_missing &e) {
                LOG_ERROR("=== Skipping dependency relation from {} to {} due "
                          "to: {}",
                    p.full_name(false), r.destination(), e.what());
            }
        }
    }

    // Process it's subpackages relationships
    for (auto subpackage = p.cbegin(); subpackage != p.cend(); subpackage++) {
        generate_relationships(**subpackage, ostr);
    }
}

void generator::generate(const package &p, std::ostream &ostr) const
{
    const auto uns = m_config.using_namespace;

    ostr << "component [" << p.name() << "] as " << p.alias();

    if (!p.style().empty())
        ostr << " " << p.style();

    ostr << " {" << '\n';

    // C++17 cannot figure out to use cbegin/cend in a for-range loop on const
    // collection...  ¯\_(ツ)_/¯
    for (auto subpackage = p.cbegin(); subpackage != p.cend(); subpackage++) {
        generate(**subpackage, ostr);
    }

    ostr << "}" << '\n';

    //
    // Process notes
    //
    for (auto decorator : p.decorators()) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(m_config.name)) {
            ostr << "note " << note->position << " of " << p.alias() << '\n'
                 << note->text << '\n'
                 << "end note\n";
        }
    }
}

void generator::generate(std::ostream &ostr) const
{
    ostr << "@startuml" << '\n';

    // Process aliases in any of the puml directives
    for (const auto &b : m_config.puml.before) {
        std::string note{b};
        std::tuple<std::string, size_t, size_t> alias_match;
        while (util::find_element_alias(note, alias_match)) {
            auto alias = m_model.to_alias(ns_relative(
                m_config.using_namespace, std::get<0>(alias_match)));
            note.replace(
                std::get<1>(alias_match), std::get<2>(alias_match), alias);
        }
        ostr << note << '\n';
    }

    if (m_config.should_include_entities("packages")) {
        for (const auto &p : m_model) {
            generate(*p, ostr);
            ostr << '\n';
        }
    }

    // Process package relationships
    for (const auto &p : m_model) {
        generate_relationships(*p, ostr);
        ostr << '\n';
    }

    // Process aliases in any of the puml directives
    for (const auto &b : m_config.puml.after) {
        std::string note{b};
        std::tuple<std::string, size_t, size_t> alias_match;
        while (util::find_element_alias(note, alias_match)) {
            auto alias = m_model.to_alias(ns_relative(
                m_config.using_namespace, std::get<0>(alias_match)));
            note.replace(
                std::get<1>(alias_match), std::get<2>(alias_match), alias);
        }
        ostr << note << '\n';
    }

    ostr << "@enduml" << '\n';
}

std::ostream &operator<<(std::ostream &os, const generator &g)
{
    g.generate(os);
    return os;
}

clanguml::package_diagram::model::diagram generate(
    cppast::libclang_compilation_database &db, const std::string &name,
    clanguml::config::package_diagram &diagram)
{
    LOG_INFO("Generating package diagram {}.puml", name);
    clanguml::package_diagram::model::diagram d;
    d.set_name(name);

    // Get all translation units matching the glob from diagram
    // configuration
    std::vector<std::string> translation_units{};
    for (const auto &g : diagram.glob) {
        LOG_DBG("Processing glob: {}", g);
        const auto matches = glob::rglob(g);
        std::copy(matches.begin(), matches.end(),
            std::back_inserter(translation_units));
    }

    cppast::cpp_entity_index idx;
    cppast::simple_file_parser<cppast::libclang_parser> parser{
        type_safe::ref(idx)};

    // Process all matching translation units
    clanguml::package_diagram::visitor::translation_unit_visitor ctx(
        idx, d, diagram);
    cppast::parse_files(parser, translation_units, db);
    for (auto &file : parser.files())
        ctx(file);

    return d;
}

}
