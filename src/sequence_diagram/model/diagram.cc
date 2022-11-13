/**
 * src/sequence_diagram/model/diagram.cc
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

#include "diagram.h"

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
    for (const auto &[id, participant] : participants) {
        if (participant->full_name(false) == full_name)
            return {*participant};
    }

    return {};
}

common::optional_ref<common::model::diagram_element> diagram::get(
    const common::model::diagram_element::id_t id) const
{
    if (participants.find(id) != participants.end())
        return {*participants.at(id)};

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
    for (const auto &[id, p] : participants) {
        elements.emplace_back(p->context());
    }

    ctx["elements"] = elements;

    return ctx;
}

void diagram::print() const
{
    LOG_DBG(" --- Participants ---");
    for (const auto &[id, participant] : participants) {
        LOG_DBG("{} - {}", id, participant->to_string());
    }

    LOG_DBG(" --- Activities ---");
    for (const auto &[from_id, act] : sequences) {
        LOG_DBG("Sequence id={}:", from_id);
        const auto &from_activity = *(participants.at(from_id));
        LOG_DBG("   Activity id={}, from={}:", act.from,
            from_activity.full_name(false));
        for (const auto &message : act.messages) {
            const auto &from_participant = *participants.at(message.from);
            const auto &to_participant = *participants.at(message.to);

            LOG_DBG("       Message from={}, from_id={}, "
                    "to={}, to_id={}, name={}",
                from_participant.full_name(false), from_participant.id(),
                to_participant.full_name(false), to_participant.id(),
                message.message_name);
        }
    }
}

}

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::sequence_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kSequence;
}
}
