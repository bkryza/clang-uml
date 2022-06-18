/**
 * src/class_diagram/generators/plantuml/class_diagram_generator.cc
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

#include "class_diagram_generator.h"

#include "util/error.h"

#include <inja/inja.hpp>

namespace clanguml::class_diagram::generators::plantuml {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_link(
    std::ostream &ostr, const class_diagram::model::class_element &e) const
{
    if (e.file().empty())
        return;

    auto context = element_context(e);

    if (!m_config.generate_links().link.empty()) {
        ostr << " [[[";
        ostr << env().render(
            std::string_view{m_config.generate_links().link}, context);
    }

    if (!m_config.generate_links().tooltip.empty()) {
        ostr << "{";
        ostr << env().render(
            std::string_view{m_config.generate_links().tooltip}, context);
        ostr << "}";
    }
    ostr << "]]]";
}

void generator::generate_alias(const class_ &c, std::ostream &ostr) const
{
    std::string class_type{"class"};
    if (c.is_abstract())
        class_type = "abstract";

    std::string full_name;
    if (m_config.generate_packages())
        full_name = c.full_name_no_ns();
    else
        full_name = c.full_name();

    if (full_name.empty())
        full_name = "<<anonymous>>";

    ostr << class_type << " \"" << full_name;

    ostr << "\" as " << c.alias() << '\n';

    // Register the added alias
    m_generated_aliases.emplace(c.alias());
}

void generator::generate_alias(const enum_ &e, std::ostream &ostr) const
{
    if (m_config.generate_packages())
        ostr << "enum"
             << " \"" << e.name();
    else
        ostr << "enum"
             << " \"" << e.full_name();

    ostr << "\" as " << e.alias() << '\n';

    // Register the added alias
    m_generated_aliases.emplace(e.alias());
}

void generator::generate(const class_ &c, std::ostream &ostr) const
{
    namespace plantuml_common = clanguml::common::generators::plantuml;

    const auto &uns = m_config.using_namespace();

    std::string class_type{"class"};
    if (c.is_abstract())
        class_type = "abstract";

    ostr << class_type << " " << c.alias();

    if (m_config.generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, c);
    }

    if (!c.style().empty())
        ostr << " " << c.style();

    ostr << " {" << '\n';

    //
    // Process methods
    //
    for (const auto &m : c.methods()) {
        if (!m_model.should_include(m.access()))
            continue;

        if (m.is_pure_virtual())
            ostr << "{abstract} ";

        if (m.is_static())
            ostr << "{static} ";

        std::string type{m.type()};

        ostr << plantuml_common::to_plantuml(m.access()) << m.name();

        ostr << "(";
        if (m_config.generate_method_arguments() !=
            config::method_arguments::none) {
            std::vector<std::string> params;
            std::transform(m.parameters().cbegin(), m.parameters().cend(),
                std::back_inserter(params), [this](const auto &mp) {
                    return mp.to_string(m_config.using_namespace());
                });
            auto args_string = fmt::format("{}", fmt::join(params, ", "));
            if (m_config.generate_method_arguments() ==
                config::method_arguments::abbreviated) {
                args_string = clanguml::util::abbreviate(args_string, 10);
            }
            ostr << args_string;
        }
        ostr << ")";

        if (m.is_const())
            ostr << " const";

        assert(!(m.is_pure_virtual() && m.is_defaulted()));

        if (m.is_pure_virtual())
            ostr << " = 0";

        if (m.is_defaulted())
            ostr << " = default";

        ostr << " : " << uns.relative(type);

        if (m_config.generate_links) {
            generate_link(ostr, m);
        }

        ostr << '\n';
    }

    //
    // Process relationships - here only generate the set of
    // rendered_relationships we'll generate them in a seperate method
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    std::set<std::string> unique_relations;
    for (const auto &r : c.relationships()) {
        if (!m_model.should_include(r.type()))
            continue;

        LOG_DBG("== Processing relationship {}",
            plantuml_common::to_plantuml(r.type(), r.style()));

        std::string destination;
        try {
            destination = r.destination();

            // TODO: Refactor destination to a namespace qualified entity
            //       name
            if (util::starts_with(destination, std::string{"::"}))
                destination = destination.substr(2, destination.size());

            LOG_DBG("=== Destination is: {}", destination);

            std::string puml_relation;
            if (!r.multiplicity_source().empty())
                puml_relation += "\"" + r.multiplicity_source() + "\" ";

            puml_relation += plantuml_common::to_plantuml(r.type(), r.style());

            if (!r.multiplicity_destination().empty())
                puml_relation += " \"" + r.multiplicity_destination() + "\"";

            if (!r.label().empty()) {
                rendered_relations.emplace(r.label());
            }
        }
        catch (error::uml_alias_missing &e) {
            LOG_DBG("=== Skipping {} relation from {} to {} due "
                    "to: {}",
                plantuml_common::to_plantuml(r.type(), r.style()),
                c.full_name(), destination, e.what());
        }
    }

    //
    // Process members
    //
    for (const auto &m : c.members()) {
        if (!m_model.should_include(m.access()))
            continue;

        if (!m_config.include_relations_also_as_members() &&
            rendered_relations.find(m.name()) != rendered_relations.end())
            continue;

        if (m.is_static())
            ostr << "{static} ";

        ostr << plantuml_common::to_plantuml(m.access()) << m.name() << " : "
             << uns.relative(m.type());

        if (m_config.generate_links) {
            generate_link(ostr, m);
        }

        ostr << '\n';
    }

    ostr << "}" << '\n';

    generate_notes(ostr, c);
}

void generator::generate_relationships(
    const class_ &c, std::ostream &ostr) const
{
    namespace plantuml_common = clanguml::common::generators::plantuml;

    const auto &uns = m_config.using_namespace();

    //
    // Process relationships
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    std::set<std::string> unique_relations;

    for (const auto &r : c.relationships()) {
        if (!m_model.should_include(r.type()))
            continue;

        LOG_DBG("== Processing relationship {}",
            plantuml_common::to_plantuml(r.type(), r.style()));

        std::stringstream relstr;
        std::string destination;
        try {
            destination = r.destination();

            // TODO: Refactor destination to a namespace qualified entity
            //       name
            if (util::starts_with(destination, std::string{"::"}))
                destination = destination.substr(2, destination.size());

            LOG_DBG("=== Destination is: {}", destination);

            std::string puml_relation;
            if (!r.multiplicity_source().empty())
                puml_relation += "\"" + r.multiplicity_source() + "\" ";

            puml_relation += plantuml_common::to_plantuml(r.type(), r.style());

            if (!r.multiplicity_destination().empty())
                puml_relation += " \"" + r.multiplicity_destination() + "\"";

            auto target_alias = m_model.to_alias(
                m_config.using_namespace().relative(destination));

            if (m_generated_aliases.find(target_alias) ==
                m_generated_aliases.end())
                continue;

            relstr << c.alias() << " " << puml_relation << " " << target_alias;

            if (!r.label().empty()) {
                relstr << " : " << plantuml_common::to_plantuml(r.access())
                       << r.label();
                rendered_relations.emplace(r.label());
            }

            if (unique_relations.count(relstr.str()) == 0) {
                unique_relations.emplace(relstr.str());

                relstr << '\n';

                LOG_DBG("=== Adding relation {}", relstr.str());

                all_relations_str << relstr.str();
            }
        }
        catch (error::uml_alias_missing &e) {
            LOG_DBG("=== Skipping {} relation from {} to {} due "
                    "to: {}",
                plantuml_common::to_plantuml(r.type(), r.style()),
                c.full_name(), destination, e.what());
        }
    }

    if (m_model.should_include(relationship_t::kExtension)) {
        for (const auto &b : c.parents()) {
            std::stringstream relstr;
            try {
                auto target_alias = m_model.to_alias(uns.relative(b.name()));

                if (m_generated_aliases.find(target_alias) ==
                    m_generated_aliases.end())
                    continue;

                relstr << target_alias << " <|-- " << c.alias() << '\n';
                all_relations_str << relstr.str();
            }
            catch (error::uml_alias_missing &e) {
                LOG_DBG("=== Skipping inheritance relation from {} to {} due "
                        "to: {}",
                    b.name(), c.name(), e.what());
            }
        }
    }

    ostr << all_relations_str.str();
}

void generator::generate(const enum_ &e, std::ostream &ostr) const
{
    ostr << "enum " << e.alias();

    if (m_config.generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, e);
    }

    if (!e.style().empty())
        ostr << " " << e.style();

    ostr << " {" << '\n';

    for (const auto &enum_constant : e.constants()) {
        ostr << enum_constant << '\n';
    }

    ostr << "}" << '\n';

    generate_notes(ostr, e);
}

void generator::generate_relationships(const enum_ &e, std::ostream &ostr) const
{
    for (const auto &r : e.relationships()) {
        if (!m_model.should_include(r.type()))
            continue;

        std::string destination;
        std::stringstream relstr;
        try {
            destination = r.destination();

            auto target_alias = m_model.to_alias(
                m_config.using_namespace().relative(destination));

            if (m_generated_aliases.find(target_alias) ==
                m_generated_aliases.end())
                continue;

            relstr << e.alias() << " "
                   << clanguml::common::generators::plantuml::to_plantuml(
                          r.type(), r.style())
                   << " " << target_alias;

            if (!r.label().empty())
                relstr << " : " << r.label();

            relstr << '\n';

            ostr << relstr.str();
        }
        catch (error::uml_alias_missing &ex) {
            LOG_DBG("Skipping {} relation from {} to {} due "
                    "to: {}",
                clanguml::common::generators::plantuml::to_plantuml(
                    r.type(), r.style()),
                e.full_name(), destination, ex.what());
        }
    }
}

void generator::generate(const package &p, std::ostream &ostr) const
{
    const auto &uns = m_config.using_namespace();

    if (m_config.generate_packages()) {
        LOG_DBG("Generating package {}", p.name());

        // Don't generate packages from namespaces filtered out by
        // using_namespace
        if (!uns.starts_with({p.full_name(false)})) {
            ostr << "package [" << p.name() << "] ";
            ostr << "as " << p.alias();

            if (p.is_deprecated())
                ostr << " <<deprecated>>";

            if (!p.style().empty())
                ostr << " " << p.style();

            ostr << " {" << '\n';
        }
    }

    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get())) {
            // TODO: add option - generate_empty_packages
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty())
                generate(sp, ostr);
        }
        else if (dynamic_cast<class_ *>(subpackage.get())) {
            if (m_model.should_include(*subpackage)) {
                generate_alias(dynamic_cast<class_ &>(*subpackage), ostr);
                generate(dynamic_cast<class_ &>(*subpackage), ostr);
            }
        }
        else if (dynamic_cast<enum_ *>(subpackage.get())) {
            if (m_model.should_include(*subpackage)) {
                generate_alias(dynamic_cast<enum_ &>(*subpackage), ostr);
                generate(dynamic_cast<enum_ &>(*subpackage), ostr);
            }
        }
    }

    if (m_config.generate_packages()) {
        // Don't generate packages from namespaces filtered out by
        // using_namespace
        if (!uns.starts_with({p.full_name(false)})) {
            ostr << "}" << '\n';
        }
    }

    generate_notes(ostr, p);
}

void generator::generate_relationships(
    const package &p, std::ostream &ostr) const
{
    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get())) {
            // TODO: add option - generate_empty_packages
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty())
                generate_relationships(sp, ostr);
        }
        else if (dynamic_cast<class_ *>(subpackage.get())) {
            if (m_model.should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<class_ &>(*subpackage), ostr);
            }
        }
        else if (dynamic_cast<enum_ *>(subpackage.get())) {
            if (m_model.should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<enum_ &>(*subpackage), ostr);
            }
        }
    }
}

void generator::generate(std::ostream &ostr) const
{
    ostr << "@startuml" << '\n';

    generate_plantuml_directives(ostr, m_config.puml().before);

    for (const auto &p : m_model) {
        if (dynamic_cast<package *>(p.get())) {
            const auto &sp = dynamic_cast<package &>(*p);
            if (!sp.is_empty())
                generate(sp, ostr);
        }
        else if (dynamic_cast<class_ *>(p.get())) {
            if (m_model.should_include(*p)) {
                generate_alias(dynamic_cast<class_ &>(*p), ostr);
                generate(dynamic_cast<class_ &>(*p), ostr);
            }
        }
        else if (dynamic_cast<enum_ *>(p.get())) {
            if (m_model.should_include(*p)) {
                generate_alias(dynamic_cast<enum_ &>(*p), ostr);
                generate(dynamic_cast<enum_ &>(*p), ostr);
            }
        }
    }

    for (const auto &p : m_model) {
        if (dynamic_cast<package *>(p.get())) {
            generate_relationships(dynamic_cast<package &>(*p), ostr);
        }
        else if (dynamic_cast<class_ *>(p.get())) {
            if (m_model.should_include(*p)) {
                generate_relationships(dynamic_cast<class_ &>(*p), ostr);
            }
        }
        else if (dynamic_cast<enum_ *>(p.get())) {
            if (m_model.should_include(*p)) {
                generate_relationships(dynamic_cast<enum_ &>(*p), ostr);
            }
        }
    }

    generate_config_layout_hints(ostr);

    generate_plantuml_directives(ostr, m_config.puml().after);

    ostr << "@enduml" << '\n';
}
}
