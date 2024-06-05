/**
 * @file src/sequence_diagram/generators/json/sequence_diagram_generator.cc
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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

namespace clanguml::sequence_diagram::generators::json {

std::string render_name(std::string name)
{
    util::replace_all(name, "##", "::");

    return name;
}

} // namespace clanguml::sequence_diagram::generators::json

namespace clanguml::sequence_diagram::model {

void to_json(nlohmann::json &j, const participant &c)
{
    to_json(j, dynamic_cast<const participant::element &>(c));
    j["type"] = c.type_name();

    if (c.type_name() == "method") {
        j["name"] = dynamic_cast<const method &>(c).method_name();
    }

    j["full_name"] = generators::json::render_name(c.full_name(false));

    if (c.type_name() == "function" || c.type_name() == "function_template") {
        const auto &f = dynamic_cast<const function &>(c);
        if (f.is_cuda_kernel())
            j["is_cuda_kernel"] = true;
        if (f.is_cuda_device())
            j["is_cuda_device"] = true;
    }
}

void to_json(nlohmann::json &j, const activity &c)
{
    j["participant_id"] = std::to_string(c.from().value());
}

} // namespace clanguml::sequence_diagram::model

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

void generator::generate_call(const message &m, nlohmann::json &parent) const
{
    const auto &from = model().get_participant<model::participant>(m.from());
    const auto &to = model().get_participant<model::participant>(m.to());

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
    else if (config().combine_free_functions_into_file_participants()) {
        if (to.value().type_name() == "function") {
            message = dynamic_cast<const model::function &>(to.value())
                          .message_name(render_mode);
        }
        else if (to.value().type_name() == "function_template") {
            message = dynamic_cast<const model::function_template &>(to.value())
                          .message_name(render_mode);
        }
    }

    message = config().simplify_template_type(message);

    nlohmann::json msg;

    msg["name"] = message;
    msg["type"] = "message";
    msg["from"]["activity_id"] = std::to_string(from.value().id().value());
    msg["to"]["activity_id"] = std::to_string(to.value().id().value());
    if (const auto &cmt = m.comment(); cmt.has_value())
        msg["comment"] = cmt.value();

    if (from.value().type_name() == "method") {
        const auto &class_participant =
            model().get_participant<model::method>(from.value().id()).value();

        msg["from"]["participant_id"] =
            std::to_string(class_participant.class_id().value());
    }
    else if (from.value().type_name() == "function" ||
        from.value().type_name() == "function_template") {
        if (config().combine_free_functions_into_file_participants()) {
            const auto &file_participant =
                model()
                    .get_participant<model::function>(from.value().id())
                    .value();
            msg["from"]["participant_id"] = std::to_string(
                common::to_id(file_participant.file_relative()).value());
        }
        else {
            msg["from"]["participant_id"] =
                std::to_string(from.value().id().value());
        }
    }
    else if (from.value().type_name() == "lambda") {
        msg["from"]["participant_id"] =
            std::to_string(from.value().id().value());
    }

    if (to.value().type_name() == "method") {
        const auto &class_participant =
            model().get_participant<model::method>(to.value().id()).value();

        msg["to"]["participant_id"] =
            std::to_string(class_participant.class_id().value());
    }
    else if (to.value().type_name() == "function" ||
        to.value().type_name() == "function_template") {
        if (config().combine_free_functions_into_file_participants()) {
            const auto &file_participant =
                model()
                    .get_participant<model::function>(to.value().id())
                    .value();
            msg["to"]["participant_id"] = std::to_string(
                common::to_id(file_participant.file_relative()).value());
        }
        else {
            msg["to"]["participant_id"] =
                std::to_string(to.value().id().value());
        }
    }
    else if (to.value().type_name() == "lambda") {
        msg["to"]["participant_id"] = std::to_string(to.value().id().value());
    }

    msg["source_location"] =
        dynamic_cast<const clanguml::common::model::source_location &>(m);

    msg["scope"] = to_string(m.message_scope());
    msg["return_type"] = config().simplify_template_type(m.return_type());

    parent["messages"].push_back(std::move(msg));

    LOG_DBG("Generated call '{}' from {} [{}] to {} [{}]", message, from,
        m.from(), to, m.to());
}

void generator::generate_activity(
    const activity &a, std::vector<eid_t> &visited) const
{
    // Generate calls from this activity to other activities
    for (const auto &m : a.messages()) {
        switch (m.type()) {
        case message_t::kCall:
            process_call_message(m, visited);
            break;
        case message_t::kIf:
            process_if_message(m);
            break;
        case message_t::kElseIf:
        case message_t::kElse:
            process_else_if_message();
            break;
        case message_t::kIfEnd:
            process_end_if_message();
            break;
        case message_t::kWhile:
            process_while_message(m);
            break;
        case message_t::kWhileEnd:
            process_end_while_message();
            break;
        case message_t::kFor:
            process_for_message(m);
            break;
        case message_t::kForEnd:
            process_end_for_message();
            break;
        case message_t::kDo:
            process_do_message(m);
            break;
        case message_t::kDoEnd:
            process_end_do_message();
            break;
        case message_t::kTry:
            process_try_message(m);
            break;
        case message_t::kCatch:
            process_catch_message();
            break;
        case message_t::kTryEnd:
            process_end_try_message();
            break;
        case message_t::kSwitch:
            process_switch_message(m);
            break;
        case message_t::kCase:
            process_case_message(m);
            break;
        case message_t::kSwitchEnd:
            process_end_switch_message();
            break;
        case message_t::kConditional:
            process_conditional_message(m);
            break;
        case message_t::kConditionalElse:
            process_conditional_else_message(m);
            break;
        case message_t::kConditionalEnd:
            process_end_conditional_message();
            break;
        case message_t::kNone:
        case message_t::kReturn:; // noop
        }
    }
}

nlohmann::json &generator::current_block_statement() const
{
    assert(!block_statements_stack_.empty());

    return block_statements_stack_.back().get();
}

void generator::process_call_message(
    const model::message &m, std::vector<eid_t> &visited) const
{
    visited.push_back(m.from());

    if (m.in_static_declaration_context()) {
        if (util::contains(already_generated_in_static_context_, m)) {
            visited.pop_back();
            return;
        }

        already_generated_in_static_context_.push_back(m);
    }

    LOG_DBG("Generating message {} --> {}", m.from(), m.to());

    generate_call(m, current_block_statement());

    if (model().sequences().find(m.to()) != model().sequences().end()) {
        if (std::find(visited.begin(), visited.end(), m.to()) ==
            visited.end()) { // break infinite recursion on recursive calls

            LOG_DBG("Creating activity {} --> {} - missing sequence {}",
                m.from(), m.to(), m.to());

            generate_activity(model().get_activity(m.to()), visited);
        }
    }
    else
        LOG_DBG("Skipping activity {} --> {} - missing sequence {}", m.from(),
            m.to(), m.to());

    visited.pop_back();
}

void generator::process_while_message(const message &m) const
{
    nlohmann::json while_block;
    while_block["type"] = "loop";
    while_block["name"] = "while";
    while_block["activity_id"] = std::to_string(m.from().value());
    if (auto text = m.condition_text(); text.has_value())
        while_block["condition_text"] = *text;

    current_block_statement()["messages"].push_back(std::move(while_block));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["messages"].back()));
}

void generator::process_end_while_message() const
{
    // Remove the while statement block from the stack
    block_statements_stack_.pop_back();
}

void generator::process_for_message(const message &m) const
{
    nlohmann::json for_block;
    for_block["type"] = "loop";
    for_block["name"] = "for";
    for_block["activity_id"] = std::to_string(m.from().value());
    if (auto text = m.condition_text(); text.has_value())
        for_block["condition_text"] = *text;

    current_block_statement()["messages"].push_back(std::move(for_block));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["messages"].back()));
}

void generator::process_end_for_message() const
{
    // Remove the while statement block from the stack
    block_statements_stack_.pop_back();
}

void generator::process_do_message(const message &m) const
{
    nlohmann::json do_block;
    do_block["type"] = "loop";
    do_block["name"] = "do";
    do_block["activity_id"] = std::to_string(m.from().value());
    if (auto text = m.condition_text(); text.has_value())
        do_block["condition_text"] = *text;

    current_block_statement()["messages"].push_back(std::move(do_block));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["messages"].back()));
}

void generator::process_end_do_message() const
{
    // Remove the do statement block from the stack
    block_statements_stack_.pop_back();
}

void generator::process_try_message(const message &m) const
{
    nlohmann::json try_block;
    try_block["type"] = "break";
    try_block["name"] = "try";
    try_block["activity_id"] = std::to_string(m.from().value());

    current_block_statement()["messages"].push_back(std::move(try_block));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["messages"].back()));

    nlohmann::json branch;
    branch["type"] = "main";
    current_block_statement()["branches"].push_back(std::move(branch));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["branches"].back()));
}

void generator::process_catch_message() const
{
    // remove previous block from the stack
    block_statements_stack_.pop_back();

    nlohmann::json branch;
    branch["type"] = "catch";
    current_block_statement()["branches"].push_back(std::move(branch));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["branches"].back()));
}

void generator::process_end_try_message() const
{
    // Remove last if block from the stack
    block_statements_stack_.pop_back();

    // Remove the try statement block from the stack
    block_statements_stack_.pop_back();
}

void generator::process_switch_message(const message &m) const
{
    nlohmann::json if_block;
    if_block["type"] = "alt";
    if_block["name"] = "switch";
    if_block["activity_id"] = std::to_string(m.from().value());

    current_block_statement()["messages"].push_back(std::move(if_block));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["messages"].back()));
}

void generator::process_case_message(const message &m) const
{
    if (current_block_statement()["type"] == "case")
        block_statements_stack_.pop_back();

    nlohmann::json case_block;
    case_block["type"] = "case";
    case_block["name"] = m.message_name();
    current_block_statement()["branches"].push_back(std::move(case_block));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["branches"].back()));
}

void generator::process_end_switch_message() const
{ // Remove last case block from the stack
    block_statements_stack_.pop_back();

    // Remove the switch statement block from the stack
    block_statements_stack_.pop_back();
}

void generator::process_conditional_message(const message &m) const
{
    nlohmann::json if_block;
    if_block["type"] = "alt";
    if_block["name"] = "conditional";
    if_block["activity_id"] = std::to_string(m.from().value());
    if (auto text = m.condition_text(); text.has_value())
        if_block["condition_text"] = *text;

    current_block_statement()["messages"].push_back(std::move(if_block));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["messages"].back()));

    nlohmann::json branch;
    branch["type"] = "consequent";
    current_block_statement()["branches"].push_back(std::move(branch));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["branches"].back()));
}

void generator::process_conditional_else_message(const message &m) const
{
    // remove previous branch from the stack
    block_statements_stack_.pop_back();

    nlohmann::json branch;
    branch["type"] = "alternative";
    if (auto text = m.condition_text(); text.has_value())
        branch["condition_text"] = *text;
    current_block_statement()["branches"].push_back(std::move(branch));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["branches"].back()));
}

void generator::process_end_conditional_message() const
{
    // Remove last if branch from the stack
    block_statements_stack_.pop_back();

    // Remove the if statement block from the stack
    block_statements_stack_.pop_back();
}

void generator::process_end_if_message() const
{
    // Remove last if branch from the stack
    block_statements_stack_.pop_back();

    // Remove the if statement block from the stack
    block_statements_stack_.pop_back();
}

void generator::process_else_if_message() const
{
    // remove previous branch from the stack
    block_statements_stack_.pop_back();

    nlohmann::json branch;
    branch["type"] = "alternative";
    current_block_statement()["branches"].push_back(std::move(branch));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["branches"].back()));
}

void generator::process_if_message(const message &m) const
{
    nlohmann::json if_block;
    if_block["type"] = "alt";
    if_block["name"] = "if";
    if_block["activity_id"] = std::to_string(m.from().value());
    if (auto text = m.condition_text(); text.has_value())
        if_block["condition_text"] = *text;

    current_block_statement()["messages"].push_back(std::move(if_block));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["messages"].back()));

    nlohmann::json branch;
    branch["type"] = "consequent";
    current_block_statement()["branches"].push_back(std::move(branch));

    block_statements_stack_.push_back(
        std::ref(current_block_statement()["branches"].back()));
}

void generator::generate_participant(
    nlohmann::json &parent, const std::string &name) const
{
    auto p = model().get(name);

    if (!p.has_value()) {
        LOG_WARN("Cannot find participant {} from `participants_order` option",
            name);
        return;
    }

    generate_participant(parent, p.value().id(), true);
}

std::optional<eid_t> generator::generate_participant(
    nlohmann::json & /*parent*/, eid_t id, bool force) const
{
    std::optional<eid_t> participant_id{};

    if (!force) {
        for (const auto pid : model().active_participants()) {
            if (pid == id) {
                participant_id = pid;
                break;
            }
        }
    }
    else
        participant_id = id;

    if (!participant_id.has_value())
        return participant_id;

    if (is_participant_generated(*participant_id))
        return participant_id;

    const auto &participant =
        model().get_participant<model::participant>(*participant_id).value();

    const auto participant_type = participant.type_name();

    if (participant_type == "method") {
        auto class_participant_id =
            model()
                .get_participant<model::method>(*participant_id)
                .value()
                .class_id();

        LOG_DBG("Generating JSON method participant: {}",
            model()
                .get_participant<model::method>(*participant_id)
                .value()
                .full_name(false));

        if (!is_participant_generated(class_participant_id)) {
            const auto &class_participant =
                model()
                    .get_participant<model::participant>(class_participant_id)
                    .value();

            generated_participants_.emplace(*participant_id);
            generated_participants_.emplace(class_participant_id);

            json_["participants"].push_back(class_participant);
            json_["participants"].back()["activities"].push_back(participant);

            // Perform config dependent postprocessing on generated class
            const auto class_participant_full_name =
                class_participant.full_name(false);

            json_["participants"].back().at("display_name") =
                make_display_name(class_participant_full_name);

            return class_participant_id;
        }

        if (!is_participant_generated(*participant_id)) {
            for (auto &p : json_["participants"]) {
                if (p.at("id") ==
                    std::to_string(class_participant_id.value())) {
                    generated_participants_.emplace(*participant_id);
                    p["activities"].push_back(participant);
                    return class_participant_id;
                }
            }
        }
    }
    else if ((participant_type == "function" ||
                 participant_type == "function_template") &&
        config().combine_free_functions_into_file_participants()) {
        // Create a single participant for all functions declared in a
        // single file
        // participant_id will become activity_id within a file participant
        const auto &function_participant =
            model().get_participant<model::function>(*participant_id).value();

        const auto file_participant_id =
            common::to_id(function_participant.file_relative());

        if (!is_participant_generated(file_participant_id)) {
            nlohmann::json p = function_participant;

            const auto file_path =
                config().make_path_relative(function_participant.file());

            p["display_name"] = util::path_to_url(file_path.string());
            p["name"] = file_path.filename();

            if (is_participant_generated(file_participant_id))
                return participant_id;

            p["id"] = std::to_string(file_participant_id.value());
            p["type"] = "file";
            p.erase("source_location");

            generated_participants_.emplace(participant_id.value());

            p["activities"].push_back(participant);
            json_["participants"].push_back(p);

            generated_participants_.emplace(file_participant_id);

            return file_participant_id;
        }

        if (!is_participant_generated(*participant_id)) {
            for (auto &p : json_["participants"]) {
                if (p.at("id") == std::to_string(file_participant_id.value())) {
                    generated_participants_.emplace(*participant_id);
                    p["activities"].push_back(participant);
                }
            }
        }

        return file_participant_id;
    }
    else {
        json_["participants"].push_back(participant);
    }

    generated_participants_.emplace(*participant_id);

    return participant_id;
}

bool generator::is_participant_generated(eid_t id) const
{
    return std::find(generated_participants_.begin(),
               generated_participants_.end(),
               id) != generated_participants_.end();
}

void generator::generate_diagram(nlohmann::json &parent) const
{
    model().print();

    if (config().using_namespace)
        parent["using_namespace"] = config().using_namespace().to_string();

    if (config().participants_order.has_value) {
        for (const auto &p : config().participants_order()) {
            LOG_DBG("Pregenerating participant {}", p);
            generate_participant(json_, p);
        }
    }

    for (const auto &ft : config().from_to()) {
        // First, find the sequence of activities from 'from' location
        // to 'to' location
        assert(ft.size() == 2);

        const auto &from_location = ft.front();
        const auto &to_location = ft.back();

        auto from_activity_id = model().get_from_activity_id(from_location);
        auto to_activity_id = model().get_to_activity_id(to_location);

        if (!from_activity_id || !to_activity_id)
            continue;

        auto message_chains_unique = model().get_all_from_to_message_chains(
            *from_activity_id, *to_activity_id);

        nlohmann::json sequence;
        sequence["from_to"]["from"]["location"] = from_location.location;
        sequence["from_to"]["from"]["id"] =
            std::to_string(from_activity_id.value().value());
        sequence["from_to"]["to"]["location"] = to_location.location;
        sequence["from_to"]["to"]["id"] =
            std::to_string(to_activity_id.value().value());

        block_statements_stack_.push_back(std::ref(sequence));

        sequence["message_chains"] = nlohmann::json::array();

        for (const auto &mc : message_chains_unique) {
            nlohmann::json message_chain;

            block_statements_stack_.push_back(std::ref(message_chain));

            for (const auto &m : mc) {
                generate_call(m, current_block_statement());
            }

            block_statements_stack_.pop_back();

            sequence["message_chains"].push_back(std::move(message_chain));
        }

        block_statements_stack_.pop_back();

        parent["sequences"].push_back(std::move(sequence));
    }

    for (const auto &to_location : config().to()) {
        auto to_activity_id = model().get_to_activity_id(to_location);

        if (!to_activity_id.has_value())
            continue;

        auto message_chains_unique = model().get_all_from_to_message_chains(
            eid_t{}, to_activity_id.value());

        nlohmann::json sequence;
        sequence["to"]["location"] = to_location.location;
        sequence["to"]["id"] = std::to_string(to_activity_id.value().value());

        block_statements_stack_.push_back(std::ref(sequence));

        sequence["message_chains"] = nlohmann::json::array();

        for (const auto &mc : message_chains_unique) {
            nlohmann::json message_chain;

            block_statements_stack_.push_back(std::ref(message_chain));

            for (const auto &m : mc) {
                generate_call(m, current_block_statement());
            }

            block_statements_stack_.pop_back();

            sequence["message_chains"].push_back(std::move(message_chain));
        }

        block_statements_stack_.pop_back();

        parent["sequences"].push_back(std::move(sequence));
    }

    for (const auto &sf : config().from()) {
        if (sf.location_type == location_t::function) {
            eid_t start_from{};
            std::string start_from_str;
            for (const auto &[k, v] : model().sequences()) {
                const auto &caller = *model().participants().at(v.from());
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
            std::vector<eid_t> visited_participants;

            const auto &from =
                model().get_participant<model::function>(start_from);

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
            sequence["start_from"]["id"] = std::to_string(start_from.value());

            block_statements_stack_.push_back(std::ref(sequence));

            generate_activity(
                model().get_activity(start_from), visited_participants);

            block_statements_stack_.pop_back();

            if (from.value().type_name() == "method" ||
                config().combine_free_functions_into_file_participants()) {

                sequence["return_type"] =
                    make_display_name(from.value().return_type());
            }

            parent["sequences"].push_back(std::move(sequence));
        }
        else {
            // TODO: Add support for other sequence start location types
            continue;
        }
    }

    // Perform config dependent postprocessing on generated participants
    for (auto &p : json_["participants"]) {
        if (p.contains("display_name")) {
            p["display_name"] = make_display_name(p["display_name"]);
        }
    }

    parent["participants"] = json_["participants"];
}

std::string generator::make_display_name(const std::string &full_name) const
{
    auto result = config().simplify_template_type(full_name);
    result = config().using_namespace().relative(result);
    common::ensure_lambda_type_is_relative(config(), result);
    result = render_name(result);

    return result;
}

} // namespace clanguml::sequence_diagram::generators::json
