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

namespace clanguml::sequence_diagram::generators::plantuml {

using clanguml::common::model::message_t;
using clanguml::config::source_location;
using clanguml::sequence_diagram::model::activity;
using clanguml::sequence_diagram::model::message;
using namespace clanguml::util;

//
// generator
//

generator::generator(
    clanguml::config::sequence_diagram &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_call(const message &m, std::ostream &ostr) const
{
    const auto from = m_config.using_namespace().relative(m.from);
    const auto to = m_config.using_namespace().relative(m.to);

    ostr << '"' << from << "\" "
         << common::generators::plantuml::to_plantuml(message_t::kCall) << " \""
         << to << "\" : " << m.message << "()" << std::endl;

    if (m.message == "add" && to == "A" && from == "A")
        LOG_DBG("Generating call '{}' from {} [{}] to {} [{}]", m.message, from,
            m.from_usr, to, m.to_usr);
    else
        LOG_DBG("Generating call '{}' from {} [{}] to {} [{}]", m.message, from,
            m.from_usr, to, m.to_usr);
}

void generator::generate_return(const message &m, std::ostream &ostr) const
{
    // Add return activity only for messages between different actors and
    // only if the return type is different than void
    if ((m.from != m.to) && (m.return_type != "void")) {
        const auto from = m_config.using_namespace().relative(m.from);
        const auto to = m_config.using_namespace().relative(m.to);

        ostr << '"' << to << "\" "
             << common::generators::plantuml::to_plantuml(message_t::kReturn)
             << " \"" << from << "\"" << std::endl;
    }
}

void generator::generate_activity(const activity &a, std::ostream &ostr) const
{
    for (const auto &m : a.messages) {
        const auto to = m_config.using_namespace().relative(m.to);
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

    generate_plantuml_directives(ostr, m_config.puml().before);

    for (const auto &sf : m_config.start_from()) {
        if (sf.location_type == source_location::location_t::function) {
            std::int64_t start_from;
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

    generate_plantuml_directives(ostr, m_config.puml().after);

    ostr << "@enduml" << std::endl;
}

}
