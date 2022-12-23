/**
 * src/sequence_diagram/model/diagram.h
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
#pragma once

#include "activity.h"
#include "common/model/diagram.h"
#include "common/types.h"
#include "participant.h"

#include <map>
#include <string>

namespace clanguml::sequence_diagram::model {

class diagram : public clanguml::common::model::diagram {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    common::model::diagram_t type() const override;

    common::optional_ref<common::model::diagram_element> get(
        const std::string &full_name) const override;

    common::optional_ref<common::model::diagram_element> get(
        common::model::diagram_element::id_t id) const override;

    std::string to_alias(const std::string &full_name) const;

    inja::json context() const override;

    void print() const;

    template <typename T>
    common::optional_ref<T> get_participant(
        common::model::diagram_element::id_t id)
    {
        if (participants_.find(id) == participants_.end()) {
            return {};
        }

        return common::optional_ref<T>(
            static_cast<T *>(participants_.at(id).get()));
    }

    template <typename T>
    common::optional_ref<T> get_participant(
        common::model::diagram_element::id_t id) const
    {
        if (participants_.find(id) == participants_.end()) {
            return {};
        }

        return common::optional_ref<T>(
            static_cast<T *>(participants_.at(id).get()));
    }

    void add_participant(std::unique_ptr<participant> p);

    void add_active_participant(common::model::diagram_element::id_t id);

    activity &get_activity(common::model::diagram_element::id_t id);

    void add_if_stmt(common::model::diagram_element::id_t current_caller_id,
        common::model::message_t type);
    void end_if_stmt(common::model::diagram_element::id_t current_caller_id,
        common::model::message_t type);

    void add_try_stmt(common::model::diagram_element::id_t current_caller_id);
    void end_try_stmt(common::model::diagram_element::id_t current_caller_id);

    void add_loop_stmt(common::model::diagram_element::id_t current_caller_id,
        common::model::message_t type);
    void end_loop_stmt(common::model::diagram_element::id_t current_caller_id,
        common::model::message_t type);

    void add_while_stmt(common::model::diagram_element::id_t current_caller_id);
    void end_while_stmt(common::model::diagram_element::id_t current_caller_id);

    void add_do_stmt(common::model::diagram_element::id_t current_caller_id);
    void end_do_stmt(common::model::diagram_element::id_t current_caller_id);

    void add_for_stmt(common::model::diagram_element::id_t current_caller_id);
    void end_for_stmt(common::model::diagram_element::id_t current_caller_id);

    void add_switch_stmt(
        common::model::diagram_element::id_t current_caller_id);
    void end_switch_stmt(
        common::model::diagram_element::id_t current_caller_id);
    void add_case_stmt(common::model::diagram_element::id_t current_caller_id);
    void add_case_stmt(common::model::diagram_element::id_t current_caller_id,
        const std::string &case_label);
    void add_default_stmt(
        common::model::diagram_element::id_t current_caller_id);

    void add_conditional_stmt(
        common::model::diagram_element::id_t current_caller_id);
    void add_conditional_elsestmt(
        common::model::diagram_element::id_t current_caller_id);
    void end_conditional_stmt(
        common::model::diagram_element::id_t current_caller_id);

    bool started() const;
    void started(bool s);

    std::map<common::model::diagram_element::id_t, activity> &sequences();

    const std::map<common::model::diagram_element::id_t, activity> &
    sequences() const;

    std::map<common::model::diagram_element::id_t, std::unique_ptr<participant>>
        &participants();

    const std::map<common::model::diagram_element::id_t,
        std::unique_ptr<participant>> &
    participants() const;

    std::set<common::model::diagram_element::id_t> &active_participants();

    const std::set<common::model::diagram_element::id_t> &
    active_participants() const;

    void add_catch_stmt(common::model::diagram_element::id_t current_caller_id,
        std::string caught_type);

private:
    void fold_or_end_block_statement(message &&m,
        common::model::message_t statement_begin,
        std::vector<message> &current_messages) const;

    bool started_{false};

    std::map<common::model::diagram_element::id_t, activity> sequences_;

    std::map<common::model::diagram_element::id_t, std::unique_ptr<participant>>
        participants_;

    std::set<common::model::diagram_element::id_t> active_participants_;
};

} // namespace clanguml::sequence_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::sequence_diagram::model::diagram>(
    diagram_t t);
}