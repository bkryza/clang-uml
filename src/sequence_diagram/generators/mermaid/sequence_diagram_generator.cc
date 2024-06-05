/**
 * @file src/sequence_diagram/generators/mermaid/sequence_diagram_generator.cc
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

namespace clanguml::sequence_diagram::generators::mermaid {

using clanguml::common::model::message_t;
using clanguml::config::location_t;
using clanguml::sequence_diagram::model::activity;
using clanguml::sequence_diagram::model::message;
using namespace clanguml::util;

using clanguml::common::generators::mermaid::indent;

std::string render_participant_name(std::string name)
{
    util::replace_all(name, "##", "::");

    return name;
}

std::string render_message_text(std::string text)
{
    util::replace_all(text, ";", "&#59;");

    return text;
}

generator::generator(
    clanguml::config::sequence_diagram &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_diagram_type(std::ostream &ostr) const
{
    ostr << "sequenceDiagram\n";
}

void generator::generate_message_comment(
    std::ostream &ostr, const model::message &m) const
{
    const auto &from = model().get_participant<model::participant>(m.from());
    if (!from)
        return;

    bool comment_generated_from_note_decorators{false};
    for (const auto &decorator : m.decorators()) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(config().name)) {
            comment_generated_from_note_decorators = true;

            ostr << indent(1) << "note over " << generate_alias(from.value())
                 << ": ";

            auto formatted_message = util::format_message_comment(
                note->text, config().message_comment_width());

            util::replace_all(formatted_message, "\n", "<br/>");
            ostr << formatted_message << '\n';
        }
    }

    if (comment_generated_from_note_decorators)
        return;

    if (const auto &cmt = m.comment();
        config().generate_message_comments() && cmt.has_value()) {

        ostr << indent(1) << "note over " << generate_alias(from.value())
             << ": ";

        auto formatted_message = util::format_message_comment(
            cmt.value(), config().message_comment_width());

        util::replace_all(formatted_message, "\n", "<br/>");

        ostr << formatted_message << '\n';
    }
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
        message = f.message_name(render_mode);
    }
    else if (config().combine_free_functions_into_file_participants()) {
        if (to.value().type_name() == "function") {
            const auto &f = dynamic_cast<const model::function &>(to.value());

            message = f.message_name(render_mode);

            if (f.is_cuda_kernel())
                message = fmt::format("<< CUDA Kernel >><br>{}", message);
            else if (f.is_cuda_device())
                message = fmt::format("<< CUDA Device >><br>{}", message);
        }
        else if (to.value().type_name() == "function_template") {
            const auto &f = dynamic_cast<const model::function &>(to.value());
            message = f.message_name(render_mode);

            if (f.is_cuda_kernel())
                message = fmt::format("<< CUDA Kernel >><br>{}", message);
            else if (f.is_cuda_device())
                message = fmt::format("<< CUDA Device >><br>{}", message);
        }
    }

    message = config().simplify_template_type(message);

    const std::string from_alias = generate_alias(from.value());
    const std::string to_alias = generate_alias(to.value());

    print_debug(m, ostr);

    generate_message_comment(ostr, m);

    ostr << indent(1) << from_alias << " "
         << common::generators::mermaid::to_mermaid(message_t::kCall) << " ";

    ostr << to_alias;

    ostr << " : ";

    if (m.message_scope() == common::model::message_scope_t::kCondition)
        ostr << "[";

    ostr << message;

    if (m.message_scope() == common::model::message_scope_t::kCondition)
        ostr << "]";

    ostr << '\n';

    LOG_DBG("Generated call '{}' from {} [{}] to {} [{}]", message, from,
        m.from(), to, m.to());
}

void generator::generate_return(const message &m, std::ostream &ostr) const
{
    // Add return activity only for messages between different actors and
    // only if the return type is different than void
    const auto &from = model().get_participant<model::participant>(m.from());
    const auto &to = model().get_participant<model::function>(m.to());
    if ((m.from() != m.to()) && !to.value().is_void()) {
        const std::string from_alias = generate_alias(from.value());

        const std::string to_alias = generate_alias(to.value());

        ostr << indent(1) << to_alias << " "
             << common::generators::mermaid::to_mermaid(message_t::kReturn)
             << " " << from_alias << " : ";

        if (config().generate_return_types()) {
            ostr << m.return_type();
        }

        ostr << '\n';
    }
}

void generator::generate_activity(
    const activity &a, std::ostream &ostr, std::vector<eid_t> &visited) const
{
    for (const auto &m : a.messages()) {
        if (m.in_static_declaration_context()) {
            if (util::contains(already_generated_in_static_context_, m))
                continue;

            already_generated_in_static_context_.push_back(m);
        }

        if (m.type() == message_t::kCall) {
            const auto &to =
                model().get_participant<model::participant>(m.to());

            visited.push_back(m.from());

            LOG_DBG("Generating message [{}] --> [{}]", m.from(), m.to());

            generate_call(m, ostr);

            std::string to_alias = generate_alias(to.value());

            ostr << indent(1) << "activate " << to_alias << '\n';

            if (model().sequences().find(m.to()) != model().sequences().end()) {
                if (std::find(visited.begin(), visited.end(), m.to()) ==
                    visited
                        .end()) { // break infinite recursion on recursive calls
                    LOG_DBG("Creating activity {} --> {} - missing sequence {}",
                        m.from(), m.to(), m.to());
                    generate_activity(
                        model().get_activity(m.to()), ostr, visited);
                }
            }
            else
                LOG_DBG("Skipping activity {} --> {} - missing sequence {}",
                    m.from(), m.to(), m.to());

            generate_return(m, ostr);

            ostr << indent(1) << "deactivate " << to_alias << '\n';

            visited.pop_back();
        }
        else if (m.type() == message_t::kIf) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << indent(1) << "alt";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << render_message_text(text.value());
            ostr << '\n';
        }
        else if (m.type() == message_t::kElseIf) {
            print_debug(m, ostr);
            ostr << indent(1) << "else";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << render_message_text(text.value());
            ostr << '\n';
        }
        else if (m.type() == message_t::kElse) {
            print_debug(m, ostr);
            ostr << indent(1) << "else\n";
        }
        else if (m.type() == message_t::kIfEnd) {
            ostr << indent(1) << "end\n";
        }
        else if (m.type() == message_t::kWhile) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << indent(1) << "loop";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << render_message_text(text.value());
            ostr << '\n';
        }
        else if (m.type() == message_t::kWhileEnd) {
            ostr << indent(1) << "end\n";
        }
        else if (m.type() == message_t::kFor) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << indent(1) << "loop";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << render_message_text(text.value());
            ostr << '\n';
        }
        else if (m.type() == message_t::kForEnd) {
            ostr << "end\n";
        }
        else if (m.type() == message_t::kDo) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << indent(1) << "loop";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << render_message_text(text.value());
            ostr << '\n';
        }
        else if (m.type() == message_t::kDoEnd) {
            ostr << indent(1) << "end\n";
        }
        else if (m.type() == message_t::kTry) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << indent(1) << "critical\n";
        }
        else if (m.type() == message_t::kCatch) {
            print_debug(m, ostr);
            ostr << indent(1) << "option "
                 << render_message_text(m.message_name()) << '\n';
        }
        else if (m.type() == message_t::kTryEnd) {
            print_debug(m, ostr);
            ostr << indent(1) << "end\n";
        }
        else if (m.type() == message_t::kSwitch) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << indent(1) << "alt\n";
        }
        else if (m.type() == message_t::kCase) {
            print_debug(m, ostr);
            ostr << indent(1) << "else "
                 << render_message_text(m.message_name()) << '\n';
        }
        else if (m.type() == message_t::kSwitchEnd) {
            ostr << indent(1) << "end\n";
        }
        else if (m.type() == message_t::kConditional) {
            print_debug(m, ostr);
            generate_message_comment(ostr, m);
            ostr << indent(1) << "alt";
            if (const auto &text = m.condition_text(); text.has_value())
                ostr << " " << render_message_text(text.value());
            ostr << '\n';
        }
        else if (m.type() == message_t::kConditionalElse) {
            print_debug(m, ostr);
            ostr << indent(1) << "else\n";
        }
        else if (m.type() == message_t::kConditionalEnd) {
            ostr << indent(1) << "end\n";
        }
    }
}

void generator::generate_participant(
    std::ostream &ostr, const std::string &name) const
{
    auto p = model().get(name);

    if (!p.has_value()) {
        LOG_WARN("Cannot find participant {} from `participants_order` option",
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

        auto participant_name =
            config().using_namespace().relative(config().simplify_template_type(
                class_participant.full_name(false)));
        common::ensure_lambda_type_is_relative(config(), participant_name);

        ostr << indent(1) << "participant " << class_participant.alias()
             << " as " << render_participant_name(participant_name);

        ostr << '\n';

        generated_participants_.emplace(class_id);
    }
    else if ((participant.type_name() == "function" ||
                 participant.type_name() == "function_template") &&
        config().combine_free_functions_into_file_participants()) {
        // Create a single participant for all functions declared in a
        // single file
        const auto &f =
            model().get_participant<model::function>(participant_id).value();

        const auto &file_path = f.file();

        assert(!file_path.empty());

        const auto file_id = common::to_id(file_path);

        if (is_participant_generated(file_id))
            return;

        auto participant_name = util::path_to_url(std::filesystem::relative(
            std::filesystem::path{file_path}, config().root_directory())
                                                      .string());

        ostr << indent(1) << "participant "
             << fmt::format("C_{:022}", file_id.value()) << " as "
             << render_participant_name(participant_name);
        ostr << '\n';

        generated_participants_.emplace(file_id);
    }
    else {
        print_debug(participant, ostr);

        auto participant_name = config().using_namespace().relative(
            config().simplify_template_type(participant.full_name(false)));
        common::ensure_lambda_type_is_relative(config(), participant_name);

        ostr << indent(1) << "participant " << participant.alias() << " as ";

        if (participant.type_name() == "function" ||
            participant.type_name() == "function_template") {
            const auto &f =
                model()
                    .get_participant<model::function>(participant_id)
                    .value();

            if (f.is_cuda_kernel())
                ostr << "<< CUDA Kernel >><br>";
            else if (f.is_cuda_device())
                ostr << "<< CUDA Device >><br>";
        }

        ostr << render_participant_name(participant_name);
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

    bool star_participant_generated{false};

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

        if (model().participants().count(*from_activity_id) == 0)
            continue;

        if (model().participants().count(*to_activity_id) == 0)
            continue;

        auto message_chains_unique = model().get_all_from_to_message_chains(
            *from_activity_id, *to_activity_id);

        for (const auto &mc : message_chains_unique) {
            const auto &from =
                model().get_participant<model::function>(*from_activity_id);

            if (from.value().type_name() == "method" ||
                config().combine_free_functions_into_file_participants()) {
                if (!star_participant_generated) {
                    ostr << indent(1) << "participant *\n";
                    star_participant_generated = true;
                }
                generate_participant(ostr, *from_activity_id);
                ostr << indent(1) << "* "
                     << common::generators::mermaid::to_mermaid(
                            message_t::kCall)
                     << " " << generate_alias(from.value()) << " : "
                     << from.value().message_name(
                            select_method_arguments_render_mode())
                     << '\n';
            }

            for (const auto &m : mc) {
                generate_call(m, ostr);
            }
        }
    }

    for (const auto &to_location : config().to()) {
        auto to_activity_id = model().get_to_activity_id(to_location);

        if (!to_activity_id)
            continue;

        auto message_chains_unique =
            model().get_all_from_to_message_chains(eid_t{}, *to_activity_id);

        for (const auto &mc : message_chains_unique) {
            const auto from_activity_id = mc.front().from();

            if (model().participants().count(from_activity_id) == 0)
                continue;

            const auto &from =
                model().get_participant<model::function>(from_activity_id);

            if (from.value().type_name() == "method" ||
                config().combine_free_functions_into_file_participants()) {
                generate_participant(ostr, from_activity_id);
                ostr << indent(1) << "* "
                     << common::generators::mermaid::to_mermaid(
                            message_t::kCall)
                     << " " << generate_alias(from.value()) << " : "
                     << from.value().message_name(
                            select_method_arguments_render_mode())
                     << '\n';
            }

            for (const auto &m : mc) {
                generate_call(m, ostr);
            }
        }
    }

    for (const auto &sf : config().from()) {
        if (sf.location_type == location_t::function) {
            eid_t start_from{};
            for (const auto &[k, v] : model().sequences()) {
                if (model().participants().count(v.from()) == 0)
                    continue;

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

            if (model().participants().count(start_from) == 0)
                continue;

            const auto &from =
                model().get_participant<model::function>(start_from);

            if (!from.has_value()) {
                LOG_WARN("Failed to find participant {} for start_from "
                         "condition",
                    sf.location);
                continue;
            }

            generate_participant(ostr, start_from);

            std::string from_alias = generate_alias(from.value());

            model::function::message_render_mode render_mode =
                select_method_arguments_render_mode();

            // For methods or functions in diagrams where they are combined into
            // file participants, we need to add an 'entry' point call to know
            // which method relates to the first activity for this 'start_from'
            // condition
            if (from.value().type_name() == "method" ||
                config().combine_free_functions_into_file_participants()) {
                ostr << indent(1) << "* "
                     << common::generators::mermaid::to_mermaid(
                            message_t::kCall)
                     << " " << from_alias << " : "
                     << from.value().message_name(render_mode) << '\n';
            }

            ostr << indent(1) << "activate " << from_alias << '\n';

            generate_activity(
                model().get_activity(start_from), ostr, visited_participants);

            if (from.value().type_name() == "method" ||
                config().combine_free_functions_into_file_participants()) {

                if (!from.value().is_void()) {
                    ostr << indent(1) << from_alias << " "
                         << common::generators::mermaid::to_mermaid(
                                message_t::kReturn)
                         << " *"
                         << " : ";

                    if (config().generate_return_types())
                        ostr << from.value().return_type();

                    ostr << '\n';
                }
            }

            ostr << indent(1) << "deactivate " << from_alias << '\n';
        }
        else {
            // TODO: Add support for other sequence start location types
            continue;
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

} // namespace clanguml::sequence_diagram::generators::mermaid
