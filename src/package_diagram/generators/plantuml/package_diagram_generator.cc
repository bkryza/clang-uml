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

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_relationships(
    const package &p, std::ostream &ostr) const
{
    LOG_DBG("Generating relationships for package {}", p.full_name(true));

    const auto &uns = m_config.using_namespace();

    // Generate this packages relationship
    if (m_config.should_include_relationship("dependency")) {
        for (const auto &r : p.relationships()) {
            std::stringstream relstr;
            try {
                relstr << m_model.to_alias(uns.relative(p.full_name(false)))
                       << " ..> "
                       << m_model.to_alias(uns.relative(r.destination()))
                       << '\n';
                ostr << relstr.str();
            }
            catch (error::uml_alias_missing &e) {
                LOG_DBG("=== Skipping dependency relation from {} to {} due "
                        "to: {}",
                    p.full_name(false), r.destination(), e.what());
            }
        }
    }

    // Process it's subpackages relationships
    for (const auto &subpackage : p) {
        generate_relationships(
            dynamic_cast<const package &>(*subpackage), ostr);
    }
}

void generator::generate(const package &p, std::ostream &ostr) const
{
    LOG_DBG("Generating package {}", p.name());

    const auto &uns = m_config.using_namespace();

    // Don't generate packages from namespaces filtered out by
    // using_namespace
    if (!uns.starts_with(p.full_name(false))) {
        ostr << "package [" << p.name() << "] ";
        ostr << "as " << p.alias();

        if (p.is_deprecated())
            ostr << " <<deprecated>>";

        if (m_config.generate_links) {
            generate_link(ostr, p);
        }

        if (!p.style().empty())
            ostr << " " << p.style();

        ostr << " {" << '\n';
    }

    for (const auto &subpackage : p) {
        generate(dynamic_cast<const package &>(*subpackage), ostr);
    }

    if (!uns.starts_with(p.full_name(false))) {
        ostr << "}" << '\n';
    }

    generate_notes(ostr, p);
}

void generator::generate(std::ostream &ostr) const
{
    ostr << "@startuml" << '\n';

    generate_plantuml_directives(ostr, m_config.puml().before);

    for (const auto &p : m_model) {
        generate(dynamic_cast<package &>(*p), ostr);
        ostr << '\n';
    }

    // Process package relationships
    for (const auto &p : m_model) {
        generate_relationships(dynamic_cast<package &>(*p), ostr);
        ostr << '\n';
    }

    generate_config_layout_hints(ostr);

    generate_plantuml_directives(ostr, m_config.puml().after);

    ostr << "@enduml" << '\n';
}
}
