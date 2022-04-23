/**
 * src/include_diagram/generators/plantuml/include_diagram_generator.cc
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

#include "include_diagram_generator.h"

#include "util/error.h"

namespace clanguml::include_diagram::generators::plantuml {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_relationships(
    const source_file &f, std::ostream &ostr) const
{
    LOG_DBG("Generating relationships for file {}", f.full_name(true));

    namespace plantuml_common = clanguml::common::generators::plantuml;

    if (f.type() == common::model::source_file_t::kDirectory) {
        for (const auto &file : f) {
            generate_relationships(
                dynamic_cast<const source_file &>(*file), ostr);
        }
    }
    else {
        for (const auto &r : f.relationships()) {
            if (m_model.should_include(r.type()) &&
                // make sure we only generate relationships for elements
                // included in the diagram
                util::contains(m_generated_aliases, r.destination()) &&
                util::contains(m_generated_aliases, f.alias())) {
                ostr << f.alias() << " "
                     << plantuml_common::to_plantuml(r.type(), r.style()) << " "
                     << r.destination() << '\n';
            }
        }
    }
}

void generator::generate(const source_file &f, std::ostream &ostr) const
{
    LOG_DBG("Generating source_file {}", f.name());

    if (f.type() == common::model::source_file_t::kDirectory) {
        ostr << "folder \"" << f.name();
        ostr << "\" as " << f.alias();
        ostr << " {\n";
        for (const auto &file : f) {
            generate(dynamic_cast<const source_file &>(*file), ostr);
        }
        ostr << "}" << '\n';

        m_generated_aliases.emplace(f.alias());
    }
    else {
        if (m_model.should_include(f)) {
            ostr << "file \"" << f.name() << "\" as " << f.alias();

            if (m_config.generate_links) {
                generate_link(ostr, f);
            }

            ostr << '\n';

            m_generated_aliases.emplace(f.alias());
        }
    }
}

void generator::generate(std::ostream &ostr) const
{
    ostr << "@startuml" << '\n';

    generate_plantuml_directives(ostr, m_config.puml().before);

    // Generate files and folders
    for (const auto &p : m_model) {
        if (p->type() == common::model::source_file_t::kDirectory ||
            m_model.should_include(*p))
            generate(dynamic_cast<source_file &>(*p), ostr);
    }

    // Process file include relationships
    for (const auto &p : m_model) {
        generate_relationships(dynamic_cast<source_file &>(*p), ostr);
    }

    generate_config_layout_hints(ostr);

    generate_plantuml_directives(ostr, m_config.puml().after);

    ostr << "@enduml" << '\n';
}
}
