/**
 * src/sequence_diagram/generators/plantuml/sequence_diagram_generator.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
using clanguml::config::location_t;
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

std::string generator::render_name(std::string name) const
{
    util::replace_all(name, "##", "::");

    return name;
}

void generator::generate_call(const message &m, std::ostream &ostr) const
{
    const auto &from = m_model.get_participant<model::participant>(m.from());
    const auto &to = m_model.get_participant<model::participant>(m.to());

    if (!from || !to) {
        LOG_DBG("Skipping empty call from '{}' to '{}'", m.from(), m.to());
        return;
    }

    generate_participant(ostr, m.from());
    generate_participant(ostr, m.to());

    std::string message;

    model::function::message_render_mode render_mode =
        model::function::message_render_mode::full;

    if (m_config.generate_method_arguments() ==
        config::method_arguments::abbreviated)
        render_mode = model::function::message_render_mode::abbreviated;
    else if (m_config.generate_method_arguments() ==
        config::method_arguments::none)
        render_mode = model::function::message_render_mode::no_arguments;

    if (to.value().type_name() == "method") {
        message = dynamic_cast<const model::function &>(to.value())
                      .message_name(render_mode);
    }
    else if (m_config.combine_free_functions_into_file_participants()) {
        if (to.value().type_name() == "function") {
            message = dynamic_cast<const model::function &>(to.value())
                          .message_name(render_mode);
        }
        else if (to.value().type_name() == "function_template") {
            message = dynamic_cast<const model::function_template &>(to.value())
                          .message_name(render_mode);
        }
    }

    const std::string from_alias = generate_alias(from.value());
    const std::string to_alias = generate_alias(to.value());

    print_debug(m, ostr);

    ostr << from_alias << " "
         << common::generators::plantuml::to_plantuml(message_t::kCall) << " ";

    ostr << to_alias;

    if (m_config.generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, m);
    }

    ostr << " : ";

    if (m.message_scope() == common::model::message_scope_t::kCondition)
        ostr << "**[**";

    ostr << message;

    if (m.message_scope() == common::model::message_scope_t::kCondition)
        ostr << "**]**";

    ostr << '\n';

    LOG_DBG("Generated call '{}' from {} [{}] to {} [{}]", message, from,
        m.from(), to, m.to());
}

void generator::generate_return(const message &m, std::ostream &ostr) const
{
    // Add return activity only for messages between different actors and
    // only if the return type is different than void
    const auto &from = m_model.get_participant<model::participant>(m.from());
    const auto &to = m_model.get_participant<model::function>(m.to());
    if ((m.from() != m.to()) && !to.value().is_void()) {
        const std::string from_alias = generate_alias(from.value());

        const std::string to_alias = generate_alias(to.value());

        ostr << to_alias << " "
             << common::generators::plantuml::to_plantuml(message_t::kReturn)
             << " " << from_alias << '\n';
    }
}

void generator::generate_activity(const activity &a, std::ostream &ostr,
    std::vector<common::model::diagram_element::id_t> &visited) const
{
    for (const auto &m : a.messages()) {
        if (m.type() == message_t::kCall) {
            const auto &to =
                m_model.get_participant<model::participant>(m.to());
            if (!to || to.value().skip())
                continue;

            visited.push_back(m.from());

            LOG_DBG("Generating message {} --> {}", m.from(), m.to());

            generate_call(m, ostr);

            std::string to_alias = generate_alias(to.value());

            ostr << "activate " << to_alias << std::endl;

            if (m_model.sequences().find(m.to()) != m_model.sequences().end()) {
                if (std::find(visited.begin(), visited.end(), m.to()) ==
                    visited
                        .end()) { // break infinite recursion on recursive calls
                    LOG_DBG("Creating activity {} --> {} - missing sequence {}",
                        m.from(), m.to(), m.to());
                    generate_activity(
                        m_model.get_activity(m.to()), ostr, visited);
                }
            }
            else
                LOG_DBG("Skipping activity {} --> {} - missing sequence {}",
                    m.from(), m.to(), m.to());

            generate_return(m, ostr);

            ostr << "deactivate " << to_alias << std::endl;

            visited.pop_back();
        }
        else if (m.type() == message_t::kIf) {
            print_debug(m, ostr);
            ostr << "alt\n";
        }
        else if (m.type() == message_t::kElseIf) {
            print_debug(m, ostr);
            ostr << "else\n";
        }
        else if (m.type() == message_t::kElse) {
            print_debug(m, ostr);
            ostr << "else\n";
        }
        else if (m.type() == message_t::kIfEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kWhile) {
            print_debug(m, ostr);
            ostr << "loop\n";
        }
        else if (m.type() == message_t::kWhileEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kFor) {
            print_debug(m, ostr);
            ostr << "loop\n";
        }
        else if (m.type() == message_t::kForEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kDo) {
            print_debug(m, ostr);
            ostr << "loop\n";
        }
        else if (m.type() == message_t::kDoEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kTry) {
            print_debug(m, ostr);
            ostr << "group try\n";
        }
        else if (m.type() == message_t::kCatch) {
            print_debug(m, ostr);
            ostr << "else " << m.message_name() << '\n';
        }
        else if (m.type() == message_t::kTryEnd) {
            print_debug(m, ostr);
            ostr << "end\n";
        }
        else if (m.type() == message_t::kSwitch) {
            print_debug(m, ostr);
            ostr << "group switch\n";
        }
        else if (m.type() == message_t::kCase) {
            print_debug(m, ostr);
            ostr << "else " << m.message_name() << '\n';
        }
        else if (m.type() == message_t::kSwitchEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kConditional) {
            print_debug(m, ostr);
            ostr << "alt\n";
        }
        else if (m.type() == message_t::kElse) {
            print_debug(m, ostr);
            ostr << "else\n";
        }
        else if (m.type() == message_t::kConditionalEnd) {
            ostr << "end\n";
        }
    }
}

void generator::generate_participant(
    std::ostream &ostr, const std::string &name) const
{
    auto p = m_model.get(name);

    if (!p.has_value()) {
        LOG_WARN("Cannot find participant {} from `participants_order` option",
            name);
        return;
    }

    generate_participant(ostr, p.value().id(), true);
}

void generator::generate_participant(
    std::ostream &ostr, common::id_t id, bool force) const
{
    common::id_t participant_id{0};

    if (!force) {
        for (const auto pid : m_model.active_participants()) {
            if (pid == id) {
                participant_id = pid;
                break;
            }
        }
    }
    else
        participant_id = id;

    if (participant_id == 0)
        return;

    if (is_participant_generated(participant_id))
        return;

    const auto &participant =
        m_model.get_participant<model::participant>(participant_id).value();

    if (participant.type_name() == "method") {
        const auto class_id =
            m_model.get_participant<model::method>(participant_id)
                .value()
                .class_id();

        if (is_participant_generated(class_id))
            return;

        const auto &class_participant =
            m_model.get_participant<model::participant>(class_id).value();

        print_debug(class_participant, ostr);

        ostr << "participant \""
             << render_name(m_config.using_namespace().relative(
                    class_participant.full_name(false)))
             << "\" as " << class_participant.alias();

        if (m_config.generate_links) {
            common_generator<diagram_config, diagram_model>::generate_link(
                ostr, class_participant);
        }

        ostr << '\n';

        generated_participants_.emplace(class_id);
    }
    else if ((participant.type_name() == "function" ||
                 participant.type_name() == "function_template") &&
        m_config.combine_free_functions_into_file_participants()) {
        // Create a single participant for all functions declared in a
        // single file
        const auto &file_path =
            m_model.get_participant<model::function>(participant_id)
                .value()
                .file();

        assert(!file_path.empty());

        const auto file_id = common::to_id(file_path);

        if (is_participant_generated(file_id))
            return;

        const auto &relative_to =
            std::filesystem::canonical(m_config.relative_to());

        auto participant_name = util::path_to_url(std::filesystem::relative(
            std::filesystem::path{file_path}, relative_to)
                                                      .string());

        ostr << "participant \"" << render_name(participant_name) << "\" as "
             << fmt::format("C_{:022}", file_id);

        ostr << '\n';

        generated_participants_.emplace(file_id);
    }
    else {
        print_debug(participant, ostr);

        ostr << "participant \""
             << m_config.using_namespace().relative(
                    participant.full_name(false))
             << "\" as " << participant.alias();

        if (m_config.generate_links) {
            common_generator<diagram_config, diagram_model>::generate_link(
                ostr, participant);
        }

        ostr << '\n';

        generated_participants_.emplace(participant_id);
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
    update_context();

    m_model.print();

    ostr << "@startuml" << std::endl;

    generate_plantuml_directives(ostr, m_config.puml().before);

    if (m_config.participants_order.has_value) {
        for (const auto &p : m_config.participants_order()) {
            LOG_DBG("Pregenerating participant {}", p);
            generate_participant(ostr, p);
        }
    }

    for (const auto &sf : m_config.start_from()) {
        if (sf.location_type == location_t::function) {
            common::model::diagram_element::id_t start_from{0};
            for (const auto &[k, v] : m_model.sequences()) {
                const auto &caller = *m_model.participants().at(v.from());
                std::string vfrom = caller.full_name(false);
                if (vfrom == sf.location) {
                    LOG_DBG("Found sequence diagram start point: {}", k);
                    start_from = k;
                    break;
                }
            }

            if (start_from == 0) {
                LOG_WARN("Failed to find participant with {} for start_from "
                         "condition",
                    sf.location);
                continue;
            }

            // Use this to break out of recurrent loops
            std::vector<common::model::diagram_element::id_t>
                visited_participants;

            const auto &from =
                m_model.get_participant<model::function>(start_from);

            if (!from.has_value()) {
                LOG_WARN("Failed to find participant {} for start_from "
                         "condition",
                    sf.location);
                continue;
            }

            generate_participant(ostr, start_from);

            std::string from_alias = generate_alias(from.value());

            model::function::message_render_mode render_mode =
                model::function::message_render_mode::full;

            if (m_config.generate_method_arguments() ==
                config::method_arguments::abbreviated)
                render_mode = model::function::message_render_mode::abbreviated;
            else if (m_config.generate_method_arguments() ==
                config::method_arguments::none)
                render_mode =
                    model::function::message_render_mode::no_arguments;

            if (from.value().type_name() == "method" ||
                m_config.combine_free_functions_into_file_participants()) {
                ostr << "[->"
                     << " " << from_alias << " : "
                     << from.value().message_name(render_mode) << std::endl;
            }

            ostr << "activate " << from_alias << std::endl;

            generate_activity(
                m_model.get_activity(start_from), ostr, visited_participants);

            if (from.value().type_name() == "method" ||
                m_config.combine_free_functions_into_file_participants()) {

                if (!from.value().is_void()) {
                    ostr << "[<--"
                         << " " << from_alias << std::endl;
                }
            }

            ostr << "deactivate " << from_alias << std::endl;
        }
        else {
            // TODO: Add support for other sequence start location types
            continue;
        }
    }

    generate_plantuml_directives(ostr, m_config.puml().after);

    ostr << "@enduml" << std::endl;
}

std::string generator::generate_alias(
    const model::participant &participant) const
{
    if ((participant.type_name() == "function" ||
            participant.type_name() == "function_template") &&
        m_config.combine_free_functions_into_file_participants()) {
        const auto file_id = common::to_id(participant.file());

        return fmt::format("C_{:022}", file_id);
    }

    return participant.alias();
}
} // namespace clanguml::sequence_diagram::generators::plantuml
