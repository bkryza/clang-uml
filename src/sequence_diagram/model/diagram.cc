/**
 * @file src/sequence_diagram/model/diagram.cc
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

#include "diagram.h"

#include "common/model/diagram_filter.h"

#include <functional>
#include <memory>

namespace clanguml::sequence_diagram::model {

common::model::diagram_t diagram::type() const
{
    return common::model::diagram_t::kSequence;
}

common::optional_ref<common::model::diagram_element> diagram::get(
    const std::string &full_name) const
{
    for (const auto &[id, participant] : participants_) {
        if (participant->full_name(false) == full_name)
            return {*participant};
    }

    return {};
}

common::optional_ref<common::model::diagram_element> diagram::get(
    const common::model::diagram_element::id_t id) const
{
    if (participants_.find(id) != participants_.end())
        return {*participants_.at(id)};

    return {};
}

std::string diagram::to_alias(const std::string &full_name) const
{
    return full_name;
}

inja::json diagram::context() const
{
    inja::json ctx;
    ctx["name"] = name();
    ctx["type"] = "sequence";

    inja::json::array_t elements{};

    // Add classes
    for (const auto &[id, p] : participants_) {
        elements.emplace_back(p->context());
    }

    ctx["elements"] = elements;

    return ctx;
}

void diagram::add_participant(std::unique_ptr<participant> p)
{
    const auto participant_id = p->id();

    if (participants_.find(participant_id) == participants_.end()) {
        LOG_DBG("Adding '{}' participant: {}, {} [{}]", p->type_name(),
            p->full_name(false), p->id(),
            p->type_name() == "method"
                ? dynamic_cast<method *>(p.get())->method_name()
                : "");

        participants_.emplace(participant_id, std::move(p));
    }
}

void diagram::add_active_participant(common::model::diagram_element::id_t id)
{
    active_participants_.emplace(id);
}

activity &diagram::get_activity(common::model::diagram_element::id_t id)
{
    return sequences_.at(id);
}

void diagram::add_message(model::message &&message)
{
    const auto caller_id = message.from();
    if (sequences_.find(caller_id) == sequences_.end()) {
        activity a{caller_id};
        sequences_.insert({caller_id, std::move(a)});
    }

    get_activity(caller_id).add_message(std::move(message));
}

void diagram::add_block_message(model::message &&message)
{
    add_message(std::move(message));
}

void diagram::end_block_message(
    model::message &&message, common::model::message_t start_type)
{
    const auto caller_id = message.from();

    if (sequences_.find(caller_id) != sequences_.end()) {
        auto &current_messages = get_activity(caller_id).messages();

        fold_or_end_block_statement(
            std::move(message), start_type, current_messages);
    }
}

void diagram::add_case_stmt_message(model::message &&m)
{
    using clanguml::common::model::message_t;
    const auto caller_id = m.from();

    if (sequences_.find(caller_id) != sequences_.end()) {
        auto &current_messages = get_activity(caller_id).messages();

        if (current_messages.back().type() == message_t::kCase) {
            // Do nothing - fallthroughs not supported yet...
        }
        else {
            current_messages.emplace_back(std::move(m));
        }
    }
}

std::map<common::model::diagram_element::id_t, activity> &diagram::sequences()
{
    return sequences_;
}

const std::map<common::model::diagram_element::id_t, activity> &
diagram::sequences() const
{
    return sequences_;
}

std::map<common::model::diagram_element::id_t, std::unique_ptr<participant>> &
diagram::participants()
{
    return participants_;
}

const std::map<common::model::diagram_element::id_t,
    std::unique_ptr<participant>> &
diagram::participants() const
{
    return participants_;
}

std::set<common::model::diagram_element::id_t> &diagram::active_participants()
{
    return active_participants_;
}

bool diagram::should_include(
    const sequence_diagram::model::participant &p) const
{
    return filter().should_include(p) &&
        filter().should_include(
            dynamic_cast<const common::model::source_location &>(p));
}

void diagram::print() const
{
    LOG_TRACE(" --- Participants ---");
    for (const auto &[id, participant] : participants_) {
        LOG_DBG("{} - {}", id, participant->to_string());
    }

    LOG_TRACE(" --- Activities ---");
    for (const auto &[from_id, act] : sequences_) {

        LOG_TRACE("Sequence id={}:", from_id);

        const auto &from_activity = *(participants_.at(from_id));

        LOG_TRACE("   Activity id={}, from={}:", act.from(),
            from_activity.full_name(false));

        for (const auto &message : act.messages()) {
            if (participants_.find(message.from()) == participants_.end())
                continue;

            const auto &from_participant = *participants_.at(message.from());

            if (participants_.find(message.to()) == participants_.end()) {
                LOG_TRACE("       Message from={}, from_id={}, "
                          "to={}, to_id={}, name={}, type={}",
                    from_participant.full_name(false), from_participant.id(),
                    "__UNRESOLVABLE_ID__", message.to(), message.message_name(),
                    to_string(message.type()));
            }
            else {
                const auto &to_participant = *participants_.at(message.to());

                LOG_TRACE("       Message from={}, from_id={}, "
                          "to={}, to_id={}, name={}, type={}",
                    from_participant.full_name(false), from_participant.id(),
                    to_participant.full_name(false), to_participant.id(),
                    message.message_name(), to_string(message.type()));
            }
        }
    }
}

void diagram::fold_or_end_block_statement(message &&m,
    const common::model::message_t statement_begin,
    std::vector<message> &current_messages) const
{
    bool is_empty_statement{true};

    auto rit = current_messages.rbegin();
    for (; rit != current_messages.rend(); rit++) {
        if (rit->type() == statement_begin) {
            break;
        }
        if (rit->type() == common::model::message_t::kCall) {
            is_empty_statement = false;
            break;
        }
    }

    if (is_empty_statement) {
        current_messages.erase((rit + 1).base(), current_messages.end());
    }
    else {
        current_messages.emplace_back(std::move(m));
    }
}

void diagram::finalize()
{
    // Apply diagram filters and remove any empty block statements
    using common::model::message_t;

    // First in each sequence (activity) filter out any remaining
    // uninteresting calls
    for (auto &[id, act] : sequences_) {
        util::erase_if(act.messages(), [this](auto &m) {
            if (m.type() != message_t::kCall)
                return false;

            const auto &to = get_participant<model::participant>(m.to());
            if (!to || to.value().skip())
                return true;

            if (!should_include(to.value())) {
                LOG_DBG("Excluding call from [{}] to {} [{}]", m.from(),
                    to.value().full_name(false), m.to());
                return true;
            }

            return false;
        });
    }

    // Now remove any empty block statements, e.g. if/endif
    for (auto &[id, act] : sequences_) {
        int64_t block_nest_level{0};
        std::vector<std::vector<message>> block_message_stack;
        // Add first stack level - this level will contain the filtered
        // message sequence
        block_message_stack.emplace_back();

        // First create a recursive stack from the messages and
        // message blocks (e.g. if statements)
        for (auto &m : act.messages()) {
            if (is_begin_block_message(m.type())) {
                block_nest_level++;
                block_message_stack.push_back({m});
            }
            else if (is_end_block_message(m.type())) {
                block_nest_level--;

                block_message_stack.back().push_back(m);

                // Check the last stack for any calls, if yes, collapse it
                // on the previous stack
                if (std::count_if(block_message_stack.back().begin(),
                        block_message_stack.back().end(), [](auto &m) {
                            return m.type() == message_t::kCall;
                        }) > 0) {
                    std::copy(block_message_stack.back().begin(),
                        block_message_stack.back().end(),
                        std::back_inserter(
                            block_message_stack.at(block_nest_level)));
                }

                block_message_stack.pop_back();

                assert(block_nest_level >= 0);
            }
            else {
                block_message_stack.back().push_back(m);
            }
        }

        act.messages().clear();

        for (auto &m : block_message_stack[0]) {
            act.add_message(m);
        }
    }
}
} // namespace clanguml::sequence_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::sequence_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kSequence;
}
}
