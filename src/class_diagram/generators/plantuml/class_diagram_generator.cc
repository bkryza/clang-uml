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

namespace clanguml::class_diagram::generators::plantuml {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
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
    namespace plantuml_common = clanguml::common::generators::plantuml;

    const auto &uns = m_config.using_namespace();

    std::string class_type{"class"};
    if (c.is_abstract())
        class_type = "abstract";

    ostr << class_type << " " << c.alias();

    if (!c.style().empty())
        ostr << " " << c.style();

    ostr << " {" << '\n';

    //
    // Process methods
    //
    for (const auto &m : c.methods()) {
        if (!m_config.should_include(m.scope()))
            continue;

        if (m.is_pure_virtual())
            ostr << "{abstract} ";

        if (m.is_static())
            ostr << "{static} ";

        std::string type{m.type()};

        ostr << plantuml_common::to_plantuml(m.scope()) << m.name();

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
        if (!m_config.should_include_relationship(
                common::model::to_string(r.type())))
            continue;

        LOG_DBG("== Processing relationship {}",
            plantuml_common::to_plantuml(r.type(), r.style()));

        std::stringstream relstr;
        std::string destination;
        try {
            destination = r.destination();

            LOG_DBG("=== Destination is: {}", destination);

            std::string puml_relation;
            if (!r.multiplicity_source().empty())
                puml_relation += "\"" + r.multiplicity_source() + "\" ";

            puml_relation += plantuml_common::to_plantuml(r.type(), r.style());

            if (!r.multiplicity_destination().empty())
                puml_relation += " \"" + r.multiplicity_destination() + "\"";

            relstr << m_model.to_alias(ns_relative(uns, c.full_name())) << " "
                   << puml_relation << " "
                   << m_model.to_alias(ns_relative(uns, destination));

            if (!r.label().empty()) {
                relstr << " : " << plantuml_common::to_plantuml(r.scope())
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
            LOG_ERROR("=== Skipping {} relation from {} to {} due "
                      "to: {}",
                plantuml_common::to_plantuml(r.type(), r.style()),
                c.full_name(), destination, e.what());
        }
    }

    //
    // Process members
    //
    for (const auto &m : c.members()) {
        if (!m_config.should_include(m.scope()))
            continue;

        if (!m_config.include_relations_also_as_members() &&
            rendered_relations.find(m.name()) != rendered_relations.end())
            continue;

        if (m.is_static())
            ostr << "{static} ";

        ostr << plantuml_common::to_plantuml(m.scope()) << m.name() << " : "
             << ns_relative(uns, m.type()) << '\n';
    }

    ostr << "}" << '\n';

    if (m_config.should_include_relationship("inheritance")) {
        for (const auto &b : c.parents()) {
            std::stringstream relstr;
            try {
                relstr << m_model.to_alias(ns_relative(uns, b.name()))
                       << " <|-- "
                       << m_model.to_alias(ns_relative(uns, c.full_name()))
                       << '\n';
                ostr << relstr.str();
            }
            catch (error::uml_alias_missing &e) {
                LOG_ERROR("=== Skipping inheritance relation from {} to {} due "
                          "to: {}",
                    b.name(), c.name(), e.what());
            }
        }
    }

    generate_notes(ostr, c);

    // Print relationships
    ostr << all_relations_str.str();
}

void generator::generate(const enum_ &e, std::ostream &ostr) const
{
    ostr << "enum " << e.alias();

    if (!e.style().empty())
        ostr << " " << e.style();

    ostr << " {" << '\n';

    for (const auto &enum_constant : e.constants()) {
        ostr << enum_constant << '\n';
    }

    ostr << "}" << '\n';

    for (const auto &r : e.relationships()) {
        if (!m_config.should_include_relationship(
                common::model::to_string(r.type())))
            continue;

        std::string destination;
        std::stringstream relstr;
        try {
            destination = r.destination();

            relstr << m_model.to_alias(
                          ns_relative(m_config.using_namespace(), e.name()))
                   << " "
                   << clanguml::common::generators::plantuml::to_plantuml(
                          r.type(), r.style())
                   << " "
                   << m_model.to_alias(
                          ns_relative(m_config.using_namespace(), destination));

            if (!r.label().empty())
                relstr << " : " << r.label();

            relstr << '\n';

            ostr << relstr.str();
        }
        catch (error::uml_alias_missing &ex) {
            LOG_ERROR("Skipping {} relation from {} to {} due "
                      "to: {}",
                clanguml::common::generators::plantuml::to_plantuml(
                    r.type(), r.style()),
                e.full_name(), destination, ex.what());
        }
    }

    generate_notes(ostr, e);
}

void generator::generate(std::ostream &ostr) const
{
    ostr << "@startuml" << '\n';

    generate_plantuml_directives(ostr, m_config.puml().before);

    if (m_config.should_include_entities("classes")) {
        for (const auto &c : m_model.classes()) {
            if (!m_config.should_include(c->get_namespace(), c->name()))
                continue;
            generate_alias(*c, ostr);
            ostr << '\n';
        }

        for (const auto &e : m_model.enums()) {
            if (!m_config.should_include(e->get_namespace(), e->name()))
                continue;
            generate_alias(*e, ostr);
            ostr << '\n';
        }

        for (const auto &c : m_model.classes()) {
            if (!m_config.should_include(c->get_namespace(), c->name()))
                continue;
            generate(*c, ostr);
            ostr << '\n';
        }
    }

    if (m_config.should_include_entities("enums"))
        for (const auto &e : m_model.enums()) {
            generate(*e, ostr);
            ostr << '\n';
        }

    generate_config_layout_hints(ostr);

    generate_plantuml_directives(ostr, m_config.puml().after);

    ostr << "@enduml" << '\n';
}

}
