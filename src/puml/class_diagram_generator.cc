/**
 * src/puml/class_diagram_generator.cc
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

#include "class_diagram_generator.h"

namespace clanguml::generators::class_diagram {

namespace puml {
std::string relative_to(std::string n, std::string c)
{
    if (c.rfind(n) == std::string::npos)
        return c;

    return c.substr(n.size() + 2);
}

//
// generator
//

generator::generator(
    clanguml::config::class_diagram &config, diagram_model &model)
    : m_config(config)
    , m_model(model)
{
}

std::string generator::to_string(scope_t scope) const
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

void generator::generate_alias(const class_ &c, std::ostream &ostr) const
{
    std::string class_type{"class"};
    if (c.is_abstract())
        class_type = "abstract";

    ostr << class_type << " \"" << c.full_name();

    ostr << "\" as " << c.alias() << '\n';
}

void generator::generate_alias(const enum_ &e, std::ostream &ostr) const
{
    ostr << "enum"
         << " \"" << e.full_name();

    ostr << "\" as " << e.alias() << '\n';
}

void generator::generate(const class_ &c, std::ostream &ostr) const
{

    const auto uns = m_config.using_namespace;

    std::string class_type{"class"};
    if (c.is_abstract())
        class_type = "abstract";

    ostr << class_type << " " << c.alias();

    if (!c.style.empty())
        ostr << " " << c.style;

    ostr << " {" << '\n';

    //
    // Process methods
    //
    for (const auto &m : c.methods) {
        if (!m_config.should_include(m.scope))
            continue;

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

        ostr << " : " << ns_relative(uns, type);

        ostr << '\n';
    }

    //
    // Process relationships
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    std::set<std::string> unique_relations;
    for (const auto &r : c.relationships()) {
        if (!m_config.should_include_relationship(name(r.type)))
            continue;

        LOG_DBG("== Processing relationship {}", to_string(r.type));

        std::stringstream relstr;
        std::string destination;
        try {
            destination = r.destination;

            LOG_DBG("=== Destination is: {}", destination);

            std::string puml_relation;
            if (!r.multiplicity_source.empty())
                puml_relation += "\"" + r.multiplicity_source + "\" ";

            puml_relation += to_string(r.type, r.style);

            if (!r.multiplicity_destination.empty())
                puml_relation += " \"" + r.multiplicity_destination + "\"";

            relstr << m_model.to_alias(ns_relative(uns, c.full_name())) << " "
                   << puml_relation << " "
                   << m_model.to_alias(ns_relative(uns, destination));

            if (!r.label.empty()) {
                relstr << " : " << to_string(r.scope) << r.label;
                rendered_relations.emplace(r.label);
            }

            if (unique_relations.count(relstr.str()) == 0) {
                unique_relations.emplace(relstr.str());

                relstr << '\n';

                LOG_DBG("=== Adding relation {}", relstr.str());

                all_relations_str << relstr.str();
            }
        }
        catch (error::uml_alias_missing &e) {
            LOG_ERROR("=== Skipping {} relation from {} to {} due "
                      "to: {}",
                to_string(r.type), c.full_name(), destination, e.what());
        }
    }

    //
    // Process members
    //
    for (const auto &m : c.members) {
        if (!m_config.should_include(m.scope))
            continue;

        if (!m_config.include_relations_also_as_members &&
            rendered_relations.find(m.name) != rendered_relations.end())
            continue;

        if (m.is_static)
            ostr << "{static} ";

        ostr << to_string(m.scope) << m.name << " : "
             << ns_relative(uns, m.type) << '\n';
    }

    ostr << "}" << '\n';

    if (m_config.should_include_relationship("inheritance"))
        for (const auto &b : c.bases) {
            std::stringstream relstr;
            try {
                relstr << m_model.to_alias(ns_relative(uns, b.name)) << " <|-- "
                       << m_model.to_alias(ns_relative(uns, c.full_name()))
                       << '\n';
                ostr << relstr.str();
            }
            catch (error::uml_alias_missing &e) {
                LOG_ERROR("=== Skipping inheritance relation from {} to {} due "
                          "to: {}",
                    b.name, c.name(), e.what());
            }
        }

    //
    // Process notes
    //
    for (auto decorator : c.decorators) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(m_config.name)) {
            ostr << "note " << note->position << " of " << c.alias() << '\n'
                 << note->text << '\n'
                 << "end note\n";
        }
    }

    // Print relationships
    ostr << all_relations_str.str();
}

void generator::generate(const enum_ &e, std::ostream &ostr) const
{
    ostr << "enum " << e.alias();

    if (!e.style.empty())
        ostr << " " << e.style;

    ostr << " {" << '\n';

    for (const auto &enum_constant : e.constants()) {
        ostr << enum_constant << '\n';
    }

    ostr << "}" << '\n';

    for (const auto &r : e.relationships()) {
        if (!m_config.should_include_relationship(name(r.type)))
            continue;

        std::string destination;
        std::stringstream relstr;
        try {

            destination = r.destination;

            relstr << m_model.to_alias(
                          ns_relative(m_config.using_namespace, e.name()))
                   << " " << to_string(r.type) << " "
                   << m_model.to_alias(
                          ns_relative(m_config.using_namespace, destination));

            if (!r.label.empty())
                relstr << " : " << r.label;

            relstr << '\n';

            ostr << relstr.str();
        }
        catch (error::uml_alias_missing &ex) {
            LOG_ERROR("Skipping {} relation from {} to {} due "
                      "to: {}",
                to_string(r.type), e.full_name(), destination, ex.what());
        }
    }

    //
    // Process notes
    //
    for (auto decorator : e.decorators) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(m_config.name)) {
            ostr << "note " << note->position << " of " << e.alias() << '\n'
                 << note->text << '\n'
                 << "end note\n";
        }
    }
}

void generator::generate(std::ostream &ostr) const
{
    ostr << "@startuml" << '\n';

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

    if (m_config.should_include_entities("classes")) {
        for (const auto &c : m_model.classes) {
            if (!c.is_template_instantiation &&
                !m_config.should_include(c.name()))
                continue;
            generate_alias(c, ostr);
            ostr << '\n';
        }

        for (const auto &e : m_model.enums) {
            if (!m_config.should_include(e.name()))
                continue;
            generate_alias(e, ostr);
            ostr << '\n';
        }

        for (const auto &c : m_model.classes) {
            if (!c.is_template_instantiation &&
                !m_config.should_include(c.name()))
                continue;
            generate(c, ostr);
            ostr << '\n';
        }
    }

    if (m_config.should_include_entities("enums"))
        for (const auto &e : m_model.enums) {
            generate(e, ostr);
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
} // namespace puml

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
        const auto matches = glob::rglob(g);
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
