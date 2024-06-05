/**
 * @file src/sequence_diagram/model/diagram.cc
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
    const eid_t id) const
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

    assert(participant_id.is_global());

    if (participants_.find(participant_id) == participants_.end()) {
        LOG_DBG("Adding '{}' participant: {}, {} [{}]", p->type_name(),
            p->full_name(false), p->id(),
            p->type_name() == "method"
                ? dynamic_cast<method *>(p.get())->method_name()
                : "");

        participants_.emplace(participant_id, std::move(p));
    }
}

void diagram::add_active_participant(eid_t id)
{
    active_participants_.emplace(id);
}

const activity &diagram::get_activity(eid_t id) const
{
    return activities_.at(id);
}

bool diagram::has_activity(eid_t id) const { return activities_.count(id) > 0; }

activity &diagram::get_activity(eid_t id) { return activities_.at(id); }

void diagram::add_message(model::message &&message)
{
    const auto caller_id = message.from();
    if (activities_.find(caller_id) == activities_.end()) {
        activity a{caller_id};
        activities_.insert({caller_id, std::move(a)});
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

    if (activities_.find(caller_id) != activities_.end()) {
        auto &current_messages = get_activity(caller_id).messages();

        fold_or_end_block_statement(
            std::move(message), start_type, current_messages);
    }
}

void diagram::add_case_stmt_message(model::message &&m)
{
    using clanguml::common::model::message_t;
    const auto caller_id = m.from();

    if (activities_.find(caller_id) != activities_.end()) {
        auto &current_messages = get_activity(caller_id).messages();

        if (current_messages.back().type() == message_t::kCase) {
            // Do nothing - fallthroughs not supported yet...
        }
        else {
            current_messages.emplace_back(std::move(m));
        }
    }
}

std::map<eid_t, activity> &diagram::sequences() { return activities_; }

const std::map<eid_t, activity> &diagram::sequences() const
{
    return activities_;
}

std::map<eid_t, std::unique_ptr<participant>> &diagram::participants()
{
    return participants_;
}

const std::map<eid_t, std::unique_ptr<participant>> &
diagram::participants() const
{
    return participants_;
}

std::set<eid_t> &diagram::active_participants() { return active_participants_; }

const std::set<eid_t> &diagram::active_participants() const
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

std::vector<std::string> diagram::list_from_values() const
{
    std::vector<std::string> result;

    for (const auto &[from_id, act] : activities_) {

        const auto &from_activity = *(participants_.at(from_id));
        const auto &full_name = from_activity.full_name(false);
        if (!full_name.empty())
            result.push_back(full_name);
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

std::vector<std::string> diagram::list_to_values() const
{
    std::vector<std::string> result;

    for (const auto &[from_id, act] : activities_) {
        for (const auto &m : act.messages()) {
            if (participants_.count(m.to()) > 0) {
                const auto &to_activity = *(participants_.at(m.to()));
                const auto &full_name = to_activity.full_name(false);
                if (!full_name.empty())
                    result.push_back(full_name);
            }
        }
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

std::optional<eid_t> diagram::get_to_activity_id(
    const config::source_location &to_location) const
{
    std::optional<eid_t> to_activity{};

    for (const auto &[k, v] : sequences()) {
        for (const auto &m : v.messages()) {
            if (m.type() != common::model::message_t::kCall)
                continue;
            const auto &callee = *participants().at(m.to());
            std::string vto = callee.full_name(false);
            if (vto == to_location.location) {
                LOG_DBG(
                    "Found sequence diagram end point '{}': {}", vto, m.to());
                to_activity = m.to();
                break;
            }
        }
    }

    if (!to_activity.has_value()) {
        LOG_WARN("Failed to find 'to' participant {} for to "
                 "condition",
            to_location.location);
    }

    return to_activity;
}

std::optional<eid_t> diagram::get_from_activity_id(
    const config::source_location &from_location) const
{
    std::optional<eid_t> from_activity{};

    for (const auto &[k, v] : sequences()) {
        const auto &caller = *participants().at(v.from());
        std::string vfrom = caller.full_name(false);
        if (vfrom == from_location.location) {
            LOG_DBG("Found sequence diagram start point '{}': {}", vfrom, k);
            from_activity = k;
            break;
        }
    }

    if (!from_activity.has_value()) {
        LOG_WARN("Failed to find 'from' participant {} for from "
                 "condition",
            from_location.location);
    }

    return from_activity;
}

std::vector<message_chain_t> diagram::get_all_from_to_message_chains(
    const eid_t from_activity, const eid_t to_activity) const
{
    std::vector<message_chain_t> message_chains_unique{};

    // Message (call) chains matching the specified from_to condition
    std::vector<message_chain_t> message_chains;

    // First find all 'to_activity' call targets in the sequences, i.e.
    // all messages pointing to the final 'to_activity' activity
    for (const auto &[k, v] : sequences()) {
        for (const auto &m : v.messages()) {
            if (m.type() != common::model::message_t::kCall)
                continue;

            if (m.to() == to_activity) {
                message_chains.emplace_back();
                message_chains.back().push_back(m);
            }
        }
    }

    std::map<unsigned int, std::vector<model::message>> calls_to_current_chain;
    std::map<unsigned int, message_chain_t> current_chain;

    int iter = 0;
    while (true) {
        bool added_message_to_some_chain{false};
        // If target of current message matches any of the
        // 'from' constraints in the last messages in
        // current chains found on previous iteration - append
        if (!calls_to_current_chain.empty()) {
            for (auto &[message_chain_index, messages] :
                calls_to_current_chain) {
                for (auto &m : messages) {
                    message_chains.push_back(
                        current_chain[message_chain_index]);

                    message_chains.back().push_back(std::move(m));
                }
            }
            calls_to_current_chain.clear();
        }

        LOG_TRACE("Message chains after iteration {}", iter++);
        int message_chain_index{};
        for (const auto &mc : message_chains) {
            LOG_TRACE("\t{}: {}", message_chain_index++,
                fmt::join(util::map<std::string>(mc,
                              [](const model::message &m) -> std::string {
                                  return m.message_name();
                              }),
                    "<-"));
        }

        for (auto i = 0U; i < message_chains.size(); i++) {
            auto &mc = message_chains[i];
            current_chain[i] = mc;
            for (const auto &[k, v] : sequences()) {
                for (const auto &m : v.messages()) {
                    if (m.type() != common::model::message_t::kCall)
                        continue;

                    // Ignore recursive calls and call loops
                    if (m.to() == m.from() ||
                        std::any_of(
                            cbegin(mc), cend(mc), [&m](const auto &msg) {
                                return msg.to() == m.from();
                            })) {
                        continue;
                    }

                    if (m.to() == mc.back().from()) {
                        calls_to_current_chain[i].push_back(m);
                        added_message_to_some_chain = true;
                    }
                }
            }

            // If there are more than one call to the current chain,
            // duplicate it as many times as there are calls - 1
            if (calls_to_current_chain.count(i) > 0 &&
                !calls_to_current_chain[i].empty()) {
                mc.push_back(calls_to_current_chain[i][0]);
                calls_to_current_chain[i].erase(
                    calls_to_current_chain[i].begin());
            }
        }

        // There is nothing more to find
        if (!added_message_to_some_chain)
            break;
    }

    // Reverse the message chains order (they were added starting from
    // the destination activity)
    for (auto &mc : message_chains) {
        std::reverse(mc.begin(), mc.end());

        if (mc.empty())
            continue;

        if (std::find(message_chains_unique.begin(),
                message_chains_unique.end(), mc) != message_chains_unique.end())
            continue;

        if (from_activity.value() == 0 ||
            (mc.front().from() == from_activity)) {
            message_chains_unique.push_back(mc);
        }
    }

    LOG_TRACE("Message chains unique", iter++);
    int message_chain_index{};
    for (const auto &mc : message_chains_unique) {
        LOG_TRACE("\t{}: {}", message_chain_index++,
            fmt::join(util::map<std::string>(mc,
                          [](const model::message &m) -> std::string {
                              return m.message_name();
                          }),
                "->"));
    }

    return message_chains_unique;
}

bool diagram::is_empty() const
{
    return activities_.empty() || participants_.empty();
}

void diagram::inline_lambda_operator_calls()
{
    std::map<eid_t, activity> activities;
    std::map<eid_t, std::unique_ptr<participant>> participants;
    std::set<eid_t> active_participants;

    for (auto &[id, act] : sequences()) {
        model::activity new_activity{id};

        // If activity is a lambda operator() - skip it
        auto maybe_lambda_activity = get_participant<model::method>(id);

        if (maybe_lambda_activity) {
            const auto parent_class_id =
                maybe_lambda_activity.value().class_id();
            auto maybe_parent_class =
                get_participant<model::class_>(parent_class_id);

            if (maybe_parent_class && maybe_parent_class.value().is_lambda()) {
                continue;
            }
        }

        // For other activities, check each message - if it calls lambda
        // operator() - reattach the message to the next activity in the chain
        // (assuming it's not lambda)
        for (auto &m : act.messages()) {

            auto message_call_to_lambda{false};

            message_call_to_lambda =
                inline_lambda_operator_call(id, new_activity, m);

            if (!message_call_to_lambda)
                new_activity.add_message(m);
        }

        // Add activity
        activities.insert({id, std::move(new_activity)});
    }

    for (auto &&[id, p] : this->participants()) {
        // Skip participants which are lambda classes
        if (const auto *maybe_class =
                dynamic_cast<const model::class_ *>(p.get());
            maybe_class != nullptr && maybe_class->is_lambda()) {
            continue;
        }

        // Skip participants which are lambda operator methods
        if (const auto *maybe_method =
                dynamic_cast<const model::method *>(p.get());
            maybe_method != nullptr) {
            auto maybe_class =
                get_participant<model::class_>(maybe_method->class_id());
            if (maybe_class && maybe_class.value().is_lambda())
                continue;
        }

        // Otherwise move the participant to the new diagram model
        auto participant_id = p->id();
        participants.emplace(participant_id, std::move(p));
    }

    // Skip active participants which are not in lambdaless_diagram participants
    for (auto id : this->active_participants()) {
        if (participants.count(id) > 0) {
            active_participants.emplace(id);
        }
    }

    activities_ = std::move(activities);
    participants_ = std::move(participants);
    active_participants_ = std::move(active_participants);
}

bool diagram::inline_lambda_operator_call(
    const eid_t id, model::activity &new_activity, const model::message &m)
{
    bool message_call_to_lambda{false};
    auto maybe_lambda_operator = get_participant<model::method>(m.to());

    if (maybe_lambda_operator) {
        const auto parent_class_id = maybe_lambda_operator.value().class_id();
        auto maybe_parent_class =
            get_participant<model::class_>(parent_class_id);

        if (maybe_parent_class && maybe_parent_class.value().is_lambda()) {
            if (has_activity(m.to())) {
                auto lambda_operator_activity = get_activity(m.to());

                // For each call in that lambda activity - reattach this
                // call to the current activity
                for (auto &mm : lambda_operator_activity.messages()) {
                    if (!inline_lambda_operator_call(id, new_activity, mm)) {
                        auto new_message{mm};

                        new_message.set_from(id);
                        new_activity.add_message(new_message);
                    }
                }
            }

            message_call_to_lambda = true;
        }
    }

    return message_call_to_lambda;
}

void diagram::print() const
{
    LOG_TRACE(" --- Participants ---");
    for (const auto &[id, participant] : participants_) {
        LOG_DBG("{} - {}", id, participant->to_string());
    }

    LOG_TRACE(" --- Activities ---");
    for (const auto &[from_id, act] : activities_) {
        if (participants_.count(from_id) == 0)
            continue;

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
                          "to={}, to_id={}, name={}, type={}, comment={}",
                    from_participant.full_name(false), from_participant.id(),
                    to_participant.full_name(false), to_participant.id(),
                    message.message_name(), to_string(message.type()),
                    message.comment().value_or("None"));
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
    for (auto &[id, act] : activities_) {
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
    for (auto &[id, act] : activities_) {
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
                if (m.type() == message_t::kCall) {
                    // Set the message return type based on the callee return
                    // type
                    auto to_participant =
                        get_participant<sequence_diagram::model::function>(
                            m.to());
                    if (to_participant.has_value()) {
                        m.set_return_type(to_participant.value().return_type());
                    }
                }
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
