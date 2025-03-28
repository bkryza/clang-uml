/**
 * @file src/sequence_diagram/generators/plantuml/sequence_diagram_generator.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include "util/error.h"
#include "util/levenshtein.h"

namespace clanguml::sequence_diagram::generators::plantuml {

using clanguml::common::eid_t;
using clanguml::common::generators::display_name_adapter;
using clanguml::common::model::message_t;
using clanguml::config::location_t;
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
    const auto &from = model().get_participant<model::participant>(m.from());
    const auto &to = model().get_participant<model::participant>(m.to());

    if (!from || !to) {
        LOG_DBG("Skipping empty call from '{}' to '{}'", m.from(), m.to());
        return;
    }

    generate_participant(ostr, m.from());
    generate_participant(ostr, m.to());

    std::string message;

    model::function::message_render_mode render_mode =
        select_method_arguments_render_mode();

    if (to.value().type_name() == "method") {
        const auto &f = dynamic_cast<const model::method &>(to.value());
        const std::string_view style = f.is_static() ? "__" : "";

        if (m.type() == message_t::kCoAwait)
            message = fmt::format("{}<< co_await >>\\n{}{}", style,
                f.message_name(render_mode), style);
        else
            message = fmt::format(
                "{}{}{}", style, f.message_name(render_mode), style);
    }
    else if (to.value().type_name() == "objc_method") {
        const auto &f = dynamic_cast<const model::objc_method &>(to.value());
        const std::string_view style = f.is_static() ? "__" : "";
        message =
            fmt::format("{}{}{}", style, f.message_name(render_mode), style);
    }
    else if (config().combine_free_functions_into_file_participants()) {
        if (to.value().type_name() == "function") {
            const auto &f = dynamic_cast<const model::function &>(to.value());
            message = f.message_name(render_mode);

            if (f.is_cuda_kernel())
                message = fmt::format("<< CUDA Kernel >>\\n{}", message);
            else if (f.is_cuda_device())
                message = fmt::format("<< CUDA Device >>\\n{}", message);
            else if (f.is_coroutine())
                message = fmt::format("<< Coroutine >>\\n{}", message);
        }
        else if (to.value().type_name() == "function_template") {
            const auto &f = dynamic_cast<const model::function &>(to.value());
            message = f.message_name(render_mode);

            if (f.is_cuda_kernel())
                message = fmt::format("<< CUDA Kernel >>\\n{}", message);
            else if (f.is_cuda_device())
                message = fmt::format("<< CUDA Device >>\\n{}", message);
            else if (f.is_coroutine())
                message = fmt::format("<< Coroutine >>\\n{}", message);
        }
    }

    message = config().simplify_template_type(message);

    const std::string from_alias = generate_alias(from.value());
    const std::string to_alias = generate_alias(to.value());

    print_debug(m, ostr);

    generate_message_comment(ostr, m);

    ostr << from_alias << " "
         << common::generators::plantuml::to_plantuml(message_t::kCall) << " ";

    ostr << to_alias;

    if (config().generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, m);
    }

    ostr << " : ";

    if (m.message_scope() == common::model::message_scope_t::kCondition)
        ostr << "**[**";

    ostr << render_message_name(message);

    if (m.message_scope() == common::model::message_scope_t::kCondition)
        ostr << "**]**";

    ostr << '\n';

    LOG_DBG("Generated call '{}' from {} [{}] to {} [{}]", message,
        from.value().full_name(false), m.from(), to.value().full_name(false),
        m.to());
}

void generator::generate_return(const message &m, std::ostream &ostr) const
{
    // Add return activity only for messages between different actors
    // and only if the return type is different than void
    if (m.from() == m.to())
        return;

    std::string message_stereotype;
    if (m.type() == message_t::kCoReturn) {
        message_stereotype = "<< co_return >>";
    }
    else if (m.type() == message_t::kCoYield) {
        message_stereotype = "<< co_yield >>";
    }

    std::string message_label;

    const auto &from = model().get_participant<model::function>(m.from());
    const auto &to = model().get_participant<model::participant>(m.to());
    if (to.has_value() && from.has_value() && !from.value().is_void()) {
        const std::string from_alias = generate_alias(from.value());

        const std::string to_alias = generate_alias(to.value());

        ostr << from_alias << " "
             << common::generators::plantuml::to_plantuml(message_t::kReturn)
             << " " << to_alias;

        if (config().generate_return_types())
            message_label = render_message_name(m.return_type());
        else if (config().generate_return_values())
            message_label = render_message_name(m.message_name());
    }
    else if (from.has_value() && !from.value().is_void() &&
        (from.value().type_name() == "method" ||
            from.value().type_name() == "objc_method" ||
            config().combine_free_functions_into_file_participants())) {
        const std::string from_alias = generate_alias(from.value());

        ostr << "[<--" << " " << from_alias;
        if (config().generate_return_types())
            message_label = render_message_name(from.value().return_type());
        else if (config().generate_return_values())
            message_label = render_message_name(m.message_name());
    }

    if (!message_stereotype.empty()) {
        if (message_label.empty())
            message_label = fmt::format("//{}//", message_stereotype);
        else
            message_label = fmt::format(
                "//{}//\\n//{}//", message_stereotype, message_label);
    }
    else {
        if (!message_label.empty())
            message_label = fmt::format("//{}//", message_label);
    }

    if (!message_label.empty())
        ostr << " : " << message_label;

    ostr << '\n';
}

void generator::generate_activity(
    eid_t activity_id, std::ostream &ostr, std::vector<eid_t> &visited) const
{
    const auto &a = model().get_activity(activity_id);

    const auto [it, inserted] = generated_activities_.emplace(activity_id);

    if (config().fold_repeated_activities() && !inserted &&
        !a.messages().empty()) {
        const auto &p =
            model().get_participant<model::participant>(activity_id);

        if (p.has_value()) {
            ostr << "hnote over " << generate_alias(p.value()) << " : *\n";
            // This is necessary to keep the hnote over the activity life line
            ostr << generate_alias(p.value()) << "-[hidden]->"
                 << generate_alias(p.value()) << '\n';
        }

        return;
    }

    for (const auto &m : a.messages()) {
        if (m.in_static_declaration_context()) {
            if (util::contains(already_generated_in_static_context_, m))
                continue;

            already_generated_in_static_context_.push_back(m);
        }

        if (m.type() == message_t::kCall || m.type() == message_t::kCoAwait) {
            const auto &to =
                model().get_participant<model::participant>(m.to());

            if (!to.has_value()) {
                LOG_DBG("Skipping activity {} due to missing target paricipant "
                        "in the diagram",
                    m.from());
                continue;
            }

            visited.push_back(m.from());

            LOG_DBG("Generating message [{}] --> [{}]", m.from(), m.to());

            generate_call(m, ostr);

            std::string to_alias = generate_alias(to.value());

            ostr << "activate " << to_alias << '\n';

            if (model().sequences().find(m.to()) != model().sequences().end()) {
                if (std::find(visited.begin(), visited.end(), m.to()) ==
                    visited
                        .end()) { // break infinite recursion on recursive calls

                    LOG_DBG("Generating activity {} (called from {})", m.to(),
                        m.from());

                    generate_activity(m.to(), ostr, visited);
                }
            }
            else
                LOG_DBG("Skipping activity {} --> {} - missing sequence {}",
                    m.from(), m.to(), m.to());

            ostr << "deactivate " << to_alias << '\n';

            visited.pop_back();
        }
        else if (m.type() == message_t::kReturn) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            auto return_message = m;
            if (!visited.empty()) {
                return_message.set_to(visited.back());
            }
            generate_return(return_message, ostr);
        }
        else if (m.type() == message_t::kCoReturn) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            auto return_message = m;
            if (!visited.empty()) {
                return_message.set_to(visited.back());
            }
            generate_return(return_message, ostr);
        }
        else if (m.type() == message_t::kCoYield) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            auto return_message = m;
            if (!visited.empty()) {
                return_message.set_to(visited.back());
            }
            generate_return(return_message, ostr);
        }
        else if (m.type() == message_t::kIf) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << "alt";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << text.value();
            ostr << '\n';
        }
        else if (m.type() == message_t::kElseIf) {
            print_debug(m, ostr);
            ostr << "else";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << text.value();
            ostr << '\n';
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
            generate_message_comment(ostr, m);
            ostr << "loop";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << text.value();
            ostr << '\n';
        }
        else if (m.type() == message_t::kWhileEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kFor) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << "loop";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << text.value();
            ostr << '\n';
        }
        else if (m.type() == message_t::kForEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kDo) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << "loop";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << text.value();
            ostr << '\n';
        }
        else if (m.type() == message_t::kDoEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kTry) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << "group try\n";
        }
        else if (m.type() == message_t::kCatch) {
            print_debug(m, ostr);
            ostr << "else " << render_message_name(m.message_name()) << '\n';
        }
        else if (m.type() == message_t::kTryEnd) {
            print_debug(m, ostr);
            ostr << "end\n";
        }
        else if (m.type() == message_t::kSwitch) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << "group switch\n";
        }
        else if (m.type() == message_t::kCase) {
            print_debug(m, ostr);
            ostr << "else " << render_message_name(m.message_name()) << '\n';
        }
        else if (m.type() == message_t::kSwitchEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kConditional) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << "alt";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << text.value();
            ostr << '\n';
        }
        else if (m.type() == message_t::kConditionalElse) {
            print_debug(m, ostr);
            ostr << "else\n";
        }
        else if (m.type() == message_t::kConditionalEnd) {
            ostr << "end\n";
        }
    }
}

void generator::generate_message_comment(
    std::ostream &ostr, const model::message &m) const
{
    const auto &from = model().get_participant<model::participant>(m.from());
    if (!from)
        return;

    // First generate message comments from \note directives in comments
    bool comment_generated_from_note_decorators{false};
    for (const auto &decorator : m.decorators()) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(config().name)) {
            comment_generated_from_note_decorators = true;

            ostr << "note over " << generate_alias(from.value()) << '\n';

            ostr << util::format_message_comment(
                        note->text, config().message_comment_width())
                 << '\n';

            ostr << "end note" << '\n';
        }
    }

    if (comment_generated_from_note_decorators)
        return;

    if (!config().generate_message_comments())
        return;

    // Now generate message notes from raw comments if enabled
    if (const auto &comment = m.comment(); comment &&
        generated_comment_ids_.emplace(comment.value().at("id")).second) {

        ostr << "note over " << generate_alias(from.value()) << '\n';

        ostr << util::format_message_comment(comment.value().at("comment"),
                    config().message_comment_width())
             << '\n';

        ostr << "end note" << '\n';
    }
}

void generator::generate_participant(
    std::ostream &ostr, const std::string &name) const
{
    auto p = model().get(name);

    if (!p.has_value()) {
        LOG_WARN("Cannot find participant {} from `participants_order` "
                 "option",
            name);
        return;
    }

    generate_participant(ostr, p.value().id(), true);
}

void generator::generate_participant(
    std::ostream &ostr, eid_t id, bool force) const
{
    eid_t participant_id{};

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

    if (participant_id == 0)
        return;

    if (is_participant_generated(participant_id))
        return;

    const auto &participant =
        model().get_participant<model::participant>(participant_id).value();

    if (participant.type_name() == "method") {
        const auto class_id =
            model()
                .get_participant<model::method>(participant_id)
                .value()
                .class_id();

        if (is_participant_generated(class_id))
            return;

        const auto &class_participant =
            model().get_participant<model::participant>(class_id).value();

        print_debug(class_participant, ostr);

        auto participant_name = config().simplify_template_type(
            display_name_adapter(class_participant).full_name(false));
        participant_name =
            config().using_namespace().relative(participant_name);

        common::ensure_lambda_type_is_relative(config(), participant_name);

        ostr << "participant \"" << participant_name << "\" as "
             << class_participant.alias();

        if (config().generate_links) {
            common_generator<diagram_config, diagram_model>::generate_link(
                ostr, class_participant);
        }

        ostr << '\n';

        generated_participants_.emplace(class_id);
    }
    else if (participant.type_name() == "objc_method") {
        const auto class_id =
            model()
                .get_participant<model::objc_method>(participant_id)
                .value()
                .class_id();

        if (is_participant_generated(class_id))
            return;

        const auto &class_participant =
            model().get_participant<model::participant>(class_id).value();

        print_debug(class_participant, ostr);

        auto participant_name = config().simplify_template_type(
            display_name_adapter(class_participant).full_name(false));
        participant_name =
            config().using_namespace().relative(participant_name);

        common::ensure_lambda_type_is_relative(config(), participant_name);

        ostr << "participant \"" << participant_name << "\" as "
             << class_participant.alias() << " <<ObjC Interface>>";

        if (config().generate_links) {
            common_generator<diagram_config, diagram_model>::generate_link(
                ostr, class_participant);
        }

        ostr << '\n';

        generated_participants_.emplace(class_id);
    }
    else if ((participant.type_name() == "function" ||
                 participant.type_name() == "function_template") &&
        config().combine_free_functions_into_file_participants()) {
        // Create a single participant for all functions declared in a
        // single file
        const auto &file_path =
            model()
                .get_participant<model::function>(participant_id)
                .value()
                .file();

        assert(!file_path.empty());

        const auto file_id = common::to_id(file_path);

        if (is_participant_generated(file_id))
            return;

        auto participant_name = util::path_to_url(relative(
            std::filesystem::path{file_path}, config().root_directory())
                                                      .string());

        ostr << "participant \"" << participant_name << "\" as "
             << fmt::format("C_{:022}", file_id.value());

        ostr << '\n';

        generated_participants_.emplace(file_id);
    }
    else {
        print_debug(participant, ostr);

        auto participant_name =
            config().using_namespace().relative(config().simplify_template_type(
                display_name_adapter(participant).full_name(false)));
        common::ensure_lambda_type_is_relative(config(), participant_name);

        ostr << "participant \"" << participant_name << "\" as "
             << participant.alias();

        if (const auto *function_ptr =
                dynamic_cast<const model::function *>(&participant);
            function_ptr) {
            if (function_ptr->is_cuda_kernel())
                ostr << " << CUDA Kernel >>";
            else if (function_ptr->is_cuda_device())
                ostr << " << CUDA Device >>";
            else if (function_ptr->is_coroutine())
                ostr << fmt::format("<< Coroutine >>");
        }

        if (config().generate_links) {
            common_generator<diagram_config, diagram_model>::generate_link(
                ostr, participant);
        }

        ostr << '\n';

        generated_participants_.emplace(participant_id);
    }
}

bool generator::is_participant_generated(eid_t id) const
{
    return std::find(generated_participants_.begin(),
               generated_participants_.end(),
               id) != generated_participants_.end();
}

std::string generator::generate_alias(
    const model::participant &participant) const
{
    if ((participant.type_name() == "function" ||
            participant.type_name() == "function_template") &&
        config().combine_free_functions_into_file_participants()) {
        const auto file_id = common::to_id(participant.file());

        return fmt::format("C_{:022}", file_id.value());
    }

    return participant.alias();
}

void generator::generate_diagram(std::ostream &ostr) const
{
    model().print();

    if (config().participants_order.has_value) {
        for (const auto &p : config().participants_order()) {
            LOG_DBG("Pregenerating participant {}", p);
            generate_participant(ostr, p);
        }
    }

    generate_from_to_sequences(ostr);

    generate_to_sequences(ostr);

    generate_from_sequences(ostr);
}

void generator::generate_from_sequences(std::ostream &ostr) const
{
    std::vector<eid_t> start_from = find_from_activities();

    // Use this to break out of recurrent loops
    std::vector<eid_t> visited_participants;

    for (const auto from_id : start_from) {
        if (model().participants().count(from_id) == 0)
            continue;

        const auto &from = model().get_participant<model::function>(from_id);

        if (!from.has_value()) {
            LOG_WARN("Failed to find participant {} for 'from' "
                     "condition");
            continue;
        }

        generate_participant(ostr, from_id);

        std::string from_alias = generate_alias(from.value());

        model::function::message_render_mode render_mode =
            select_method_arguments_render_mode();

        // For methods or functions in diagrams where they are
        // combined into file participants, we need to add an
        // 'entry' point call to know which method relates to the
        // first activity for this 'start_from' condition
        if (from.value().type_name() == "method" ||
            from.value().type_name() == "objc_method" ||
            config().combine_free_functions_into_file_participants()) {
            ostr << "[->" << " " << from_alias << " : "
                 << render_message_name(from.value().message_name(render_mode))
                 << '\n';
        }

        ostr << "activate " << from_alias << '\n';

        generate_activity(from_id, ostr, visited_participants);

        ostr << "deactivate " << from_alias << '\n';
    }
}

std::vector<eid_t> generator::find_from_activities() const
{
    std::vector<eid_t> start_from;
    for (const auto &sf : config().from()) {
        if (sf.location_type == location_t::function) {
            bool found{false};
            for (const auto &[k, v] : model().sequences()) {
                if (model().participants().count(v.from()) == 0)
                    continue;

                const auto &caller = *model().participants().at(v.from());
                std::string vfrom = caller.full_name(false);
                if (sf.location == vfrom) {
                    LOG_DBG("Found sequence diagram start point: {}", k);
                    start_from.push_back(k);
                    found = true;
                }
            }

            if (!found) {
                model().handle_invalid_from_condition(sf);
            }
        }
    }

    return start_from;
}

std::vector<model::message_chain_t> generator::find_to_message_chains() const
{
    std::vector<model::message_chain_t> result;

    for (const auto &to_location : config().to()) {
        auto to_activity_ids = model().get_to_activity_ids(to_location);

        if (to_activity_ids.empty()) {
            model().handle_invalid_to_condition(to_location);
        }

        for (const auto &to_activity_id : to_activity_ids) {
            std::vector<model::message_chain_t> message_chains_unique =
                model().get_all_from_to_message_chains(eid_t{}, to_activity_id);

            result.insert(result.end(), message_chains_unique.begin(),
                message_chains_unique.end());
        }
    }

    return result;
}

std::string generator::render_message_name(const std::string &m) const
{
    return util::abbreviate(m, config().message_name_width());
}

void generator::generate_to_sequences(std::ostream &ostr) const
{
    std::vector<model::message_chain_t> message_chains =
        find_to_message_chains();

    bool first_separator_skipped{false};
    for (const auto &mc : message_chains) {
        if (!first_separator_skipped)
            first_separator_skipped = true;
        else
            ostr << "====\n";

        const auto from_activity_id = mc.front().from();

        if (model().participants().count(from_activity_id) == 0)
            continue;

        const auto &from =
            model().get_participant<model::function>(from_activity_id);

        if (from.value().type_name() == "method" ||
            from.value().type_name() == "objc_method" ||
            config().combine_free_functions_into_file_participants()) {
            generate_participant(ostr, from_activity_id);
            ostr << "[->" << " " << generate_alias(from.value()) << " : "
                 << render_message_name(from.value().message_name(
                        select_method_arguments_render_mode()))
                 << '\n';
        }

        for (const auto &m : mc) {
            generate_call(m, ostr);
        }
    }
}

void generator::generate_from_to_sequences(std::ostream &ostr) const
{
    for (const auto &ft : config().from_to()) {
        // First, find the sequence of activities from 'from' location
        // to 'to' location
        assert(ft.size() == 2);

        const auto &from_location = ft.front();
        const auto &to_location = ft.back();

        const auto from_activity_ids =
            model().get_from_activity_ids(from_location);

        const auto to_activity_ids = model().get_to_activity_ids(to_location);

        if (from_activity_ids.empty()) {
            model().handle_invalid_from_condition(from_location);
        }

        if (from_activity_ids.empty() || to_activity_ids.empty()) {
            model().handle_invalid_to_condition(to_location);
        }

        bool first_separator_skipped{false};

        for (const auto from_activity_id : from_activity_ids) {
            if (model().participants().count(from_activity_id) == 0)
                continue;

            for (const auto to_activity_id : to_activity_ids) {
                if (model().participants().count(to_activity_id) == 0)
                    continue;

                auto message_chains_unique =
                    model().get_all_from_to_message_chains(
                        from_activity_id, to_activity_id);

                for (const auto &mc : message_chains_unique) {
                    if (!first_separator_skipped)
                        first_separator_skipped = true;
                    else
                        ostr << "====\n";

                    const auto &from = model().get_participant<model::function>(
                        from_activity_id);

                    if (from.value().type_name() == "method" ||
                        from.value().type_name() == "objc_method" ||
                        config()
                            .combine_free_functions_into_file_participants()) {
                        generate_participant(ostr, from_activity_id);
                        ostr << "[->" << " " << generate_alias(from.value())
                             << " : "
                             << render_message_name(from.value().message_name(
                                    select_method_arguments_render_mode()))
                             << '\n';
                    }

                    for (const auto &m : mc) {
                        generate_call(m, ostr);
                    }
                }
            }
        }
    }
}

model::function::message_render_mode
generator::select_method_arguments_render_mode() const
{
    if (config().generate_method_arguments() ==
        config::method_arguments::abbreviated)
        return model::function::message_render_mode::abbreviated;

    if (config().generate_method_arguments() == config::method_arguments::none)
        return model::function::message_render_mode::no_arguments;

    return model::function::message_render_mode::full;
}

} // namespace clanguml::sequence_diagram::generators::plantuml
