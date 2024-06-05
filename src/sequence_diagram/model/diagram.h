/**
 * @file src/sequence_diagram/model/diagram.h
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
#pragma once

#include "activity.h"
#include "common/model/diagram.h"
#include "common/types.h"
#include "config/config.h"
#include "participant.h"

#include <map>
#include <string>

namespace clanguml::sequence_diagram::model {

using message_chain_t = std::vector<sequence_diagram::model::message>;

/**
 * @brief Model of a sequence diagram
 *
 * @embed{sequence_model_class.svg}
 */
class diagram : public clanguml::common::model::diagram {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    /**
     * @brief Get the diagram model type - in this case sequence.
     *
     * @return Type of sequence diagram.
     */
    common::model::diagram_t type() const override;

    /**
     * @brief Search for element in the diagram by fully qualified name.
     *
     * @param full_name Fully qualified element name.
     * @return Optional reference to a diagram element.
     */
    common::optional_ref<common::model::diagram_element> get(
        const std::string &full_name) const override;

    /**
     * @brief Search for element in the diagram by id.
     *
     * @param id Element id.
     * @return Optional reference to a diagram element.
     */
    common::optional_ref<common::model::diagram_element> get(
        eid_t id) const override;

    /**
     * @brief Get participant by id
     *
     * @param id Participant id.
     * @return Optional reference to a diagram element.
     */
    template <typename T>
    common::optional_ref<T> get_participant(eid_t id) const
    {
        if (participants_.find(id) == participants_.end()) {
            return {};
        }

        return common::optional_ref<T>(
            dynamic_cast<T *>(participants_.at(id).get()));
    }

    /**
     * @brief Add sequence diagram participant
     *
     * @param p Sequence diagram participant model
     */
    void add_participant(std::unique_ptr<participant> p);

    /**
     * @brief Set participant with `id` as active
     *
     * @param id Id of participant to activate
     */
    void add_active_participant(eid_t id);

    /**
     * @brief Check if diagram has activity identified by caller id
     *
     * @param id Caller id representing the activity
     * @return True, if an activity already exists
     */
    bool has_activity(eid_t id) const;

    /**
     * @brief Get reference to current activity of a participant
     *
     * @param id Participant id
     * @return
     */
    const activity &get_activity(eid_t id) const;

    /**
     * @brief Get reference to current activity of a participant
     *
     * @param id Participant id
     * @return
     */
    activity &get_activity(eid_t id);

    /**
     * @brief Add message to current activity
     *
     * @param message Message model
     */
    void add_message(model::message &&message);

    /**
     * @brief Add block message to the current activity
     *
     * Block messages represent sequence diagram blocks such as `alt`
     * or `loop`.
     *
     * The block messages can be stacked.
     *
     * @param message Message model
     */
    void add_block_message(model::message &&message);

    /**
     * @brief End current block message
     *
     * @param message Message model
     * @param start_type Type of block statement.
     */
    void end_block_message(
        model::message &&message, common::model::message_t start_type);

    /**
     * @brief Add `switch` block `case` statement
     * @param m Message model
     */
    void add_case_stmt_message(model::message &&m);

    /**
     * @brief Get all sequences in the diagram
     *
     * @return Map of sequences in the diagram
     */
    std::map<eid_t, activity> &sequences();

    /**
     * @brief Get all sequences in the diagram
     *
     * @return Map of sequences in the diagram
     */
    const std::map<eid_t, activity> &sequences() const;

    /**
     * @brief Get map of all participants in the diagram
     *
     * @return Map of participants in the diagram
     */
    std::map<eid_t, std::unique_ptr<participant>> &participants();

    /**
     * @brief Get map of all participants in the diagram
     *
     * @return Map of participants in the diagram
     */
    const std::map<eid_t, std::unique_ptr<participant>> &participants() const;

    /**
     * @brief Get all active participants in the diagram
     *
     * @return Set of all active participant ids
     */
    std::set<eid_t> &active_participants();

    /**
     * @brief Get all active participants in the diagram
     *
     * @return Set of all active participant ids
     */
    const std::set<eid_t> &active_participants() const;

    /**
     * @brief Convert element full name to PlantUML alias.
     *
     * @todo This method does not belong here - refactor to PlantUML specific
     *       code.
     *
     * @param full_name Full name of the diagram element.
     * @return PlantUML alias.
     */
    std::string to_alias(const std::string &full_name) const;

    /**
     * @brief Return the elements JSON context for inja templates.
     *
     * @return JSON node with elements context.
     */
    inja::json context() const override;

    /**
     * @brief Debug method for printing entire diagram to console.
     */
    void print() const;

    // Implicitly import should_include overloads from base class
    using common::model::diagram::should_include;

    /**
     * @brief Convenience `should_include` overload for participant
     * @param p Participant model
     * @return True, if the participant should be included in the diagram
     */
    bool should_include(const sequence_diagram::model::participant &p) const;

    /**
     * @brief Get list of all possible 'from' values in the model
     *
     * @return List of all possible 'from' values
     */
    std::vector<std::string> list_from_values() const;

    /**
     * @brief Get list of all possible 'to' values in the model
     *
     * @return List of all possible 'to' values
     */
    std::vector<std::string> list_to_values() const;

    /**
     * @brief Generate a list of message chains matching a from_to constraint
     *
     * If 'from_activity' is 0, this method will return all message chains
     * ending in 'to_activity'.
     *
     * @param from_activity Source activity for from_to message chain
     * @param to_activity Target activity for from_to message chain
     * @return List of message chains
     */
    std::vector<message_chain_t> get_all_from_to_message_chains(
        eid_t from_activity, eid_t to_activity) const;

    /**
     * @brief Get id of a 'to' activity
     *
     * @param to_location Target activity
     * @return Activity id
     */
    std::optional<eid_t> get_to_activity_id(
        const config::source_location &to_location) const;

    /**
     * @brief Get id of a 'from' activity
     *
     * @param from_location Source activity
     * @return Activity id
     */
    std::optional<eid_t> get_from_activity_id(
        const config::source_location &from_location) const;

    /**
     * @brief Once the diagram is complete, run any final processing.
     *
     * This method should be overriden by specific diagram models to do some
     * final tasks like cleaning up the model (e.g. some filters only work
     * on completed diagrams).
     */
    void finalize() override;

    /**
     * @brief Check whether the diagram is empty
     *
     * @return True, if diagram is empty
     */
    bool is_empty() const override;

    /**
     * If option to inline lambda calls is enabled, we need to modify the
     * sequences to skip the lambda calls. In case lambda call does not lead
     * to a non-lambda call, omit it entirely
     */
    void inline_lambda_operator_calls();

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

    bool is_begin_block_message(common::model::message_t mt)
    {
        using common::model::message_t;
        static std::set<message_t> block_begin_types{message_t::kIf,
            message_t::kWhile, message_t::kDo, message_t::kFor, message_t::kTry,
            message_t::kSwitch, message_t::kConditional};

        return block_begin_types.count(mt) > 0;
    };

    bool is_end_block_message(common::model::message_t mt)
    {
        using common::model::message_t;
        static std::set<message_t> block_end_types{message_t::kIfEnd,
            message_t::kWhileEnd, message_t::kDoEnd, message_t::kForEnd,
            message_t::kTryEnd, message_t::kSwitchEnd,
            message_t::kConditionalEnd};

        return block_end_types.count(mt) > 0;
    };

    bool inline_lambda_operator_call(
        eid_t id, model::activity &new_activity, const model::message &m);

    std::map<eid_t, activity> activities_;

    std::map<eid_t, std::unique_ptr<participant>> participants_;

    std::set<eid_t> active_participants_;
};

} // namespace clanguml::sequence_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::sequence_diagram::model::diagram>(
    diagram_t t);
}