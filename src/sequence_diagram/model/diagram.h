/**
 * src/sequence_diagram/model/diagram.h
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

    void add_message(model::message &&message);

    void add_block_message(model::message &&message);

    void end_block_message(
        model::message &&message, common::model::message_t start_type);

    void add_case_stmt_message(model::message &&m);

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

private:
    /**
     * This method checks the last messages in sequence (current_messages),
     * if they represent a block sequence identified by statement_begin
     * (e.g. if/else) and there are no actual call expressions within this block
     * statement the entire block statement is removed from the end of the
     * sequence.
     *
     * Otherwise the block statement is ended with a proper statement
     * (e.g. endif)
     *
     * @param m Message to add to the sequence
     * @param statement_begin Type of message which begins this type of block
     *                        statement (e.g. message_t::kIf)
     * @param current_messages Reference to the sequence messages which should
     *                         be amended
     */
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