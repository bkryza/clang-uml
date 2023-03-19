/**
 * src/sequence_diagram/generators/json/sequence_diagram_generator.cc
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

namespace clanguml::sequence_diagram::model {
using nlohmann::json;

void to_json(nlohmann::json &j, const participant &c)
{
    j["name"] = c.full_name(false);
    j["id"] = std::to_string(c.id());
    j["type"] = c.type_name();
    if (!c.file().empty())
        j["source_location"] =
            dynamic_cast<const common::model::source_location &>(c);
    if (const auto &comment = c.comment(); comment)
        j["comment"] = comment.value();
}

void to_json(nlohmann::json &j, const activity &c)
{
    j["participant_id"] = std::to_string(c.from());
}

} // namespace clanguml::class_diagram::model

namespace clanguml::sequence_diagram::generators::json {

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

void generator::generate_call(const message &m, nlohmann::json &parent) const
{
    const auto &from = m_model.get_participant<model::participant>(m.from());
    const auto &to = m_model.get_participant<model::participant>(m.to());

    if (!from || !to) {
        LOG_DBG("Skipping empty call from '{}' to '{}'", m.from(), m.to());
        return;
    }

    generate_participant(json_, m.from());
    generate_participant(json_, m.to());

    std::string message;

    model::function::message_render_mode render_mode =
        model::function::message_render_mode::full;

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
    //
    //    const std::string from_alias = generate_alias(from.value());
    //    const std::string to_alias = generate_alias(to.value());

    nlohmann::json msg;

    msg["name"] = message;
    msg["type"] = "message";
    msg["from"]["name"] = from.value().full_name(false);
    msg["from"]["id"] = std::to_string(from.value().id());
    msg["to"]["activity_id"] = std::to_string(to.value().id());
    msg["to"]["activity_name"] = to.value().full_name(false);

    if (to.value().type_name() == "method") {
        const auto &class_participant =
            m_model.get_participant<model::method>(to.value().id()).value();
        msg["to"]["participant_id"] =
            std::to_string(class_participant.class_id());
        msg["to"]["participant_name"] =
            m_model.get_participant<model::class_>(class_participant.class_id())
                .value()
                .full_name(false);
    }
    else if (to.value().type_name() == "function" &&
        m_config.combine_free_functions_into_file_participants()) {
        const auto &file_participant =
            m_model.get_participant<model::function>(to.value().id()).value();
        msg["to"]["participant_id"] =
            std::to_string(common::to_id(file_participant.file()));
        msg["to"]["participant_name"] = file_participant.file_relative();
    }

    msg["source_location"] =
        dynamic_cast<const clanguml::common::model::source_location &>(m);

    msg["scope"] = to_string(m.message_scope());
    msg["return_type"] = m.return_type();

    parent["messages"].push_back(std::move(msg));

    LOG_DBG("Generated call '{}' from {} [{}] to {} [{}]", message, from,
        m.from(), to, m.to());
}

void generator::generate_activity(
    common::model::diagram_element::id_t activity_id, const activity &a,
    nlohmann::json &unused,
    std::vector<common::model::diagram_element::id_t> &visited,
    std::optional<nlohmann::json> nested_block) const
{
    // Generate calls from this activity to other activities
    for (const auto &m : a.messages()) {
        if (m.type() == message_t::kCall) {
            const auto &to =
                m_model.get_participant<model::participant>(m.to());
            if (!to || to.value().skip())
                continue;

            visited.push_back(m.from());

            LOG_DBG("Generating message {} --> {}", m.from(), m.to());

            generate_call(m, current_block_statement());

            if (m_model.sequences().find(m.to()) != m_model.sequences().end()) {
                if (std::find(visited.begin(), visited.end(), m.to()) ==
                    visited
                        .end()) { // break infinite recursion on recursive calls

                    LOG_DBG("Creating activity {} --> {} - missing sequence {}",
                        m.from(), m.to(), m.to());

                    generate_activity(m.to(), m_model.get_activity(m.to()),
                        current_block_statement(), visited, {});
                }
            }
            else
                LOG_DBG("Skipping activity {} --> {} - missing sequence {}",
                    m.from(), m.to(), m.to());
        }
        else if (m.type() == message_t::kIf) {
            nlohmann::json if_block;
            if_block["type"] = "alt";
            if_block["name"] = "if";
            if_block["activity_id"] = std::to_string(m.from());

            current_block_statement()["messages"].push_back(
                std::move(if_block));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["messages"].back()));

            nlohmann::json branch;
            branch["type"] = "consequent";
            current_block_statement()["branches"].push_back(std::move(branch));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["branches"].back()));
        }
        else if (m.type() == message_t::kElseIf ||
            m.type() == message_t::kElse) {
            // remove previous branch from the stack
            block_statements_stack_.pop_back();

            nlohmann::json branch;
            branch["type"] = "alternative";
            current_block_statement()["branches"].push_back(std::move(branch));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["branches"].back()));
        }
        else if (m.type() == message_t::kIfEnd) {
            // Remove last if branch from the stack
            block_statements_stack_.pop_back();

            // Remove the if statement block from the stack
            block_statements_stack_.pop_back();
        }
        else if (m.type() == message_t::kWhile) {
            nlohmann::json while_block;
            while_block["type"] = "loop";
            while_block["name"] = "while";
            while_block["activity_id"] = std::to_string(m.from());

            current_block_statement()["messages"].push_back(
                std::move(while_block));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["messages"].back()));
        }
        else if (m.type() == message_t::kWhileEnd) {
            // Remove the while statement block from the stack
            block_statements_stack_.pop_back();
        }
        else if (m.type() == message_t::kFor) {
            nlohmann::json for_block;
            for_block["type"] = "loop";
            for_block["name"] = "for";
            for_block["activity_id"] = std::to_string(m.from());

            current_block_statement()["messages"].push_back(
                std::move(for_block));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["messages"].back()));
        }
        else if (m.type() == message_t::kForEnd) {
            // Remove the while statement block from the stack
            block_statements_stack_.pop_back();
        }
        else if (m.type() == message_t::kDo) {
            nlohmann::json do_block;
            do_block["type"] = "loop";
            do_block["name"] = "do";
            do_block["activity_id"] = std::to_string(m.from());

            current_block_statement()["messages"].push_back(
                std::move(do_block));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["messages"].back()));
        }
        else if (m.type() == message_t::kDoEnd) {
            // Remove the do statement block from the stack
            block_statements_stack_.pop_back();
        }
        else if (m.type() == message_t::kTry) {
            nlohmann::json try_block;
            try_block["type"] = "break";
            try_block["name"] = "try";
            try_block["activity_id"] = std::to_string(m.from());

            current_block_statement()["messages"].push_back(
                std::move(try_block));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["messages"].back()));

            nlohmann::json branch;
            branch["type"] = "main";
            current_block_statement()["blocks"].push_back(std::move(branch));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["blocks"].back()));
        }
        else if (m.type() == message_t::kCatch) {
            // remove previous block from the stack
            block_statements_stack_.pop_back();

            nlohmann::json branch;
            branch["type"] = "catch";
            current_block_statement()["blocks"].push_back(std::move(branch));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["blocks"].back()));
        }
        else if (m.type() == message_t::kTryEnd) {
            // Remove last if block from the stack
            block_statements_stack_.pop_back();

            // Remove the try statement block from the stack
            block_statements_stack_.pop_back();
        }
        else if (m.type() == message_t::kSwitch) {
            nlohmann::json if_block;
            if_block["type"] = "alt";
            if_block["name"] = "switch";
            if_block["activity_id"] = std::to_string(m.from());

            current_block_statement()["messages"].push_back(
                std::move(if_block));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["messages"].back()));
        }
        else if (m.type() == message_t::kCase) {
            if (current_block_statement()["type"] == "case")
                block_statements_stack_.pop_back();

            nlohmann::json case_block;
            case_block["type"] = "case";
            case_block["name"] = m.message_name();
            current_block_statement()["cases"].push_back(std::move(case_block));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["cases"].back()));
        }
        else if (m.type() == message_t::kSwitchEnd) {
            // Remove last case block from the stack
            block_statements_stack_.pop_back();

            // Remove the switch statement block from the stack
            block_statements_stack_.pop_back();
        }
        else if (m.type() == message_t::kConditional) {
            nlohmann::json if_block;
            if_block["type"] = "alt";
            if_block["name"] = "conditional";
            if_block["activity_id"] = std::to_string(m.from());

            current_block_statement()["messages"].push_back(
                std::move(if_block));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["messages"].back()));

            nlohmann::json branch;
            branch["type"] = "consequent";
            current_block_statement()["branches"].push_back(std::move(branch));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["branches"].back()));
        }
        else if (m.type() == message_t::kElse) {
            // remove previous branch from the stack
            block_statements_stack_.pop_back();

            nlohmann::json branch;
            branch["type"] = "alternative";
            current_block_statement()["branches"].push_back(std::move(branch));

            block_statements_stack_.push_back(
                std::ref(current_block_statement()["branches"].back()));
        }
        else if (m.type() == message_t::kConditionalEnd) {
            // Remove last if branch from the stack
            block_statements_stack_.pop_back();

            // Remove the if statement block from the stack
            block_statements_stack_.pop_back();
        }
        else {
            // Unhandled message_t case
            assert(false);
        }
    }
}

void generator::generate_participant(
    nlohmann::json &parent, const std::string &name) const
{
    auto p = m_model.get(name);

    if (!p.has_value()) {
        LOG_WARN("Cannot find participant {} from `participants_order` option",
            name);
        return;
    }

    generate_participant(parent, p.value().id(), true);
}

common::id_t generator::generate_participant(
    nlohmann::json &parent, common::id_t id, bool force) const
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
        return participant_id;

    if (is_participant_generated(participant_id))
        return participant_id;

    const auto &participant =
        m_model.get_participant<model::participant>(participant_id).value();

    if (participant.type_name() == "method") {
        const auto class_id =
            m_model.get_participant<model::method>(participant_id)
                .value()
                .class_id();

        if (is_participant_generated(class_id))
            return participant_id;

        const auto &class_participant =
            m_model.get_participant<model::participant>(class_id).value();

        parent["participants"].push_back(class_participant);

        generated_participants_.emplace(class_id);
    }
    else if ((participant.type_name() == "function" ||
                 participant.type_name() == "function_template") &&
        m_config.combine_free_functions_into_file_participants()) {
        // Create a single participant for all functions declared in a
        // single file
        const auto &function_participant =
            m_model.get_participant<model::function>(participant_id).value();

        parent["participants"].push_back(function_participant);

        generated_participants_.emplace(
            common::to_id(function_participant.file()));
    }
    else {
        parent["participants"].push_back(participant);

        generated_participants_.emplace(participant_id);
    }

    return participant_id;
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

    json_["name"] = m_model.name();
    json_["diagram_type"] = "sequence";
    if (m_config.using_namespace)
        json_["using_namespace"] = m_config.using_namespace().to_string();

    if (m_config.participants_order.has_value) {
        for (const auto &p : m_config.participants_order()) {
            LOG_DBG("Pregenerating participant {}", p);
            generate_participant(json_, p);
        }
    }

    for (const auto &sf : m_config.start_from()) {
        if (sf.location_type == location_t::function) {
            common::model::diagram_element::id_t start_from{0};
            std::string start_from_str;
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

            generate_participant(json_, start_from);

            [[maybe_unused]] model::function::message_render_mode render_mode =
                model::function::message_render_mode::full;

            nlohmann::json sequence;
            sequence["start_from"]["location"] = sf.location;
            sequence["start_from"]["id"] = start_from;

            block_statements_stack_.push_back(std::ref(sequence));

            generate_activity(start_from, m_model.get_activity(start_from),
                sequence, visited_participants, {});

            json_["sequences"].push_back(std::move(sequence));

            block_statements_stack_.pop_back();

            if (from.value().type_name() == "method" ||
                m_config.combine_free_functions_into_file_participants()) {

                // TODO
                // sequence["return_type"] = from.value().id()
            }
        }
        else {
            // TODO: Add support for other sequence start location types
            continue;
        }
    }

    ostr << json_;
}
} // namespace clanguml::sequence_diagram::generators::plantuml
