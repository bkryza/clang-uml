/**
 * @file src/sequence_diagram/generators/json/sequence_diagram_generator.h
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

#include "common/generators/json/generator.h"
#include "config/config.h"
#include "sequence_diagram/model/diagram.h"
#include "util/util.h"

#include <glob/glob.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

namespace clanguml::sequence_diagram::generators::json {

using clanguml::common::eid_t;

std::string render_name(std::string name);

using diagram_config = clanguml::config::sequence_diagram;
using diagram_model = clanguml::sequence_diagram::model::diagram;

template <typename C, typename D>
using common_generator = clanguml::common::generators::json::generator<C, D>;

/**
 * @brief Sequence diagram JSON generator
 */
class generator : public common_generator<diagram_config, diagram_model> {
public:
    generator(diagram_config &config, diagram_model &model);

    using common_generator<diagram_config, diagram_model>::generate;

    /**
     * @brief Main generator method.
     *
     * This method is called first and coordinates the entire diagram
     * generation.
     *
     * @param ostr Output stream.
     */
    void generate_diagram(nlohmann::json &parent) const override;

    /**
     * @brief Generate sequence diagram message.
     *
     * @param m Message model
     * @param parent JSON node
     */
    void generate_call(const sequence_diagram::model::message &m,
        nlohmann::json &parent) const;

    /**
     * @brief Generate sequence diagram participant
     *
     * @param parent JSON node
     * @param id Participant id
     * @param force If true, generate the participant even if its not in
     *              the set of active participants
     * @return Id of the generated participant
     */
    std::optional<eid_t> generate_participant(
        nlohmann::json &parent, eid_t id, bool force = false) const;

    /**
     * @brief Generate sequence diagram participant by name
     *
     * This is convienience wrapper over `generate_participant()` by id.
     *
     * @param parent JSON node
     * @param name Full participant name
     */
    void generate_participant(
        nlohmann::json &parent, const std::string &name) const;

    /**
     * @brief Generate sequence diagram activity.
     *
     * @param a Activity model
     * @param visited List of already visited participants, this is necessary
     *                for breaking infinite recursion on recursive calls
     */
    void generate_activity(const sequence_diagram::model::activity &a,
        std::vector<eid_t> &visited) const;

    /**
     * @brief Get reference to the current block statement.
     *
     * This method returns a reference to the last block statement (e.g if
     * statement or for loop) in the call stack.
     *
     * @return Reference to the current block statement.
     */
    nlohmann::json &current_block_statement() const;

    std::string make_display_name(const std::string &full_name) const;

private:
    /**
     * @brief Check if specified participant has already been generated.
     *
     * @param id Participant id.
     * @return True, if participant has already been generated.
     */
    bool is_participant_generated(eid_t id) const;

    /**
     * @brief Process call message
     *
     * @param m Message model
     * @param visited List of already visited participants
     */
    void process_call_message(
        const model::message &m, std::vector<eid_t> &visited) const;

    /**
     * @brief Process `if` statement message
     *
     * @param m Message model
     */
    void process_if_message(const model::message &m) const;

    /**
     * @brief Process `else if` statement message
     */
    void process_else_if_message() const;

    /**
     * @brief Process `end if` statement message
     */
    void process_end_if_message() const;

    /**
     * @brief Process `:?` statement message
     *
     * @param m Message model
     */
    void process_conditional_message(const model::message &m) const;

    /**
     * @brief Process end of conditional statement message
     */
    void process_end_conditional_message() const;

    /**
     * @brief Process conditional else statement message
     *
     * @param m Message model
     */
    void process_conditional_else_message(const model::message &m) const;

    /**
     * @brief Process `switch` statement message
     *
     * @param m Message model
     */
    void process_switch_message(const model::message &m) const;

    /**
     * @brief Process switch end statement message
     */
    void process_end_switch_message() const;

    /**
     * @brief Process `switch` `case` statement message
     *
     * @param m Message model
     */
    void process_case_message(const model::message &m) const;

    /**
     * @brief Process `try` statement message
     *
     * @param m Message model
     */
    void process_try_message(const model::message &m) const;

    /**
     * @brief Process `try` end statement message
     */
    void process_end_try_message() const;

    /**
     * @brief Process `catch` statement message
     */
    void process_catch_message() const;

    /**
     * @brief Process `do` loop statement message
     *
     * @param m Message model
     */
    void process_do_message(const model::message &m) const;

    /**
     * @brief Process `do` end statement message
     */
    void process_end_do_message() const;

    /**
     * @brief Process `for` loop statement message
     *
     * @param m Message model
     */
    void process_for_message(const model::message &m) const;

    /**
     * @brief Process `for` end statement message
     */
    void process_end_for_message() const;

    /**
     * @brief Process `while` loop message
     *
     * @param m Message model
     */
    void process_while_message(const model::message &m) const;

    /**
     * @brief Process `while` end loop message
     */
    void process_end_while_message() const;

    mutable std::set<eid_t> generated_participants_;

    // Needed to add "participants" array in a temporary object accessible from
    // all methods of the generator
    mutable nlohmann::json json_;

    mutable std::vector<std::reference_wrapper<nlohmann::json>>
        block_statements_stack_;

    mutable std::vector<model::message> already_generated_in_static_context_;
};

} // namespace clanguml::sequence_diagram::generators::json
