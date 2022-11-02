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
    const auto &from = m_model.get_participant<model::participant>(m.from);
    const auto &to = m_model.get_participant<model::participant>(m.to);

    if (!from || !to) {
        LOG_DBG("Skipping empty call from '{}' to '{}'", m.from, m.to);
        return;
    }

    generate_participant(ostr, m.from);
    generate_participant(ostr, m.to);

    auto message = m.message_name;
    if (!message.empty()) {
        message = m_config.using_namespace().relative(message);
        message += "()";
    }

    ostr << from.value().alias() << " "
         << common::generators::plantuml::to_plantuml(message_t::kCall) << " "
         << to.value().alias() << " : " << message << std::endl;

    LOG_DBG("Generated call '{}' from {} [{}] to {} [{}]", message, from,
        m.from_name, to, m.to_name);
}

void generator::generate_return(const message &m, std::ostream &ostr) const
{
    // Add return activity only for messages between different actors and
    // only if the return type is different than void
    if ((m.from != m.to) && (m.return_type != "void")) {
        const auto &from = m_model.get_participant<model::participant>(m.from);
        const auto &to = m_model.get_participant<model::participant>(m.to);

        ostr << to.value().alias() << " "
             << common::generators::plantuml::to_plantuml(message_t::kReturn)
             << " " << from.value().alias() << '\n';
    }
}

void generator::generate_activity(const activity &a, std::ostream &ostr) const
{
    for (const auto &m : a.messages) {
        const auto &to = m_model.get_participant<model::participant>(m.to);
        if (!to)
            continue;

        LOG_DBG("Generating message {} --> {}", m.from_name, m.to_name);
        generate_call(m, ostr);

        ostr << "activate " << to.value().alias() << std::endl;

        if (m_model.sequences.find(m.to) != m_model.sequences.end()) {
            LOG_DBG("Creating activity {} --> {} - missing sequence {}",
                m.from_name, m.to_name, m.to);
            generate_activity(m_model.sequences[m.to], ostr);
        }
        else
            LOG_DBG("Skipping activity {} --> {} - missing sequence {}",
                m.from_name, m.to_name, m.to);

        generate_return(m, ostr);

        ostr << "deactivate " << to.value().alias() << std::endl;
    }
}

void generator::generate_participant(std::ostream &ostr, common::id_t id) const
{
    for (const auto participant_id : m_model.active_participants_) {
        if (participant_id != id)
            continue;

        if (is_participant_generated(participant_id))
            return;

        const auto &participant =
            m_model.get_participant<model::participant>(participant_id).value();

        if (participant.type_name() == "method") {
            const auto &class_id =
                m_model.get_participant<model::method>(participant_id)
                    .value()
                    .class_id();

            if (is_participant_generated(class_id))
                return;

            const auto &class_participant =
                m_model.get_participant<model::participant>(class_id).value();

            ostr << "participant \""
                 << m_config.using_namespace().relative(
                        class_participant.full_name(false))
                 << "\" as " << class_participant.alias() << '\n';

            generated_participants_.emplace(class_id);
        }
        else {
            ostr << "participant \""
                 << m_config.using_namespace().relative(
                        participant.full_name(false))
                 << "\" as " << participant.alias() << '\n';

            generated_participants_.emplace(participant_id);
        }

        return;
    }
}

bool generator::is_participant_generated(common::id_t id) const
{
    return std::find(generated_participants_.begin(),
               generated_participants_.end(),
               id) != generated_participants_.end();
}

void generator::generate(std::ostream &ostr) const
{
    m_model.print();

    ostr << "@startuml" << std::endl;

    generate_plantuml_directives(ostr, m_config.puml().before);

    for (const auto &sf : m_config.start_from()) {
        if (sf.location_type == source_location::location_t::function) {
            std::int64_t start_from;
            for (const auto &[k, v] : m_model.sequences) {
                std::string vfrom = v.from;
                if (vfrom == sf.location) {
                    LOG_DBG("Found sequence diagram start point: {}", k);
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
