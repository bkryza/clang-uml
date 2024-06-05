/**
 * @file src/sequence_diagram/generators/plantuml/sequence_diagram_generator.h
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

#include "common/generators/plantuml/generator.h"
#include "config/config.h"
#include "sequence_diagram/model/diagram.h"
#include "sequence_diagram/visitor/translation_unit_visitor.h"
#include "util/util.h"

#include <glob/glob.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace sequence_diagram {
namespace generators {
namespace plantuml {

using clanguml::common::eid_t;

using diagram_config = clanguml::config::sequence_diagram;
using diagram_model = clanguml::sequence_diagram::model::diagram;

template <typename C, typename D>
using common_generator =
    clanguml::common::generators::plantuml::generator<C, D>;

/**
 * @brief Sequence diagram PlantUML generator
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
    void generate_diagram(std::ostream &ostr) const override;

    /**
     * @brief Generate sequence diagram message.
     *
     * @param m Message model
     * @param ostr Output stream
     */
    void generate_call(const clanguml::sequence_diagram::model::message &m,
        std::ostream &ostr) const;

    /**
     * @brief Generate sequence diagram return message
     *
     * @param m Message model
     * @param ostr Output stream
     */
    void generate_return(const clanguml::sequence_diagram::model::message &m,
        std::ostream &ostr) const;

    /**
     * @brief Generate sequence diagram participant
     *
     * @param ostr Output stream
     * @param id Participant id
     * @param force If true, generate the participant even if its not in
     *              the set of active participants
     * @return Id of the generated participant
     */
    void generate_participant(
        std::ostream &ostr, eid_t id, bool force = false) const;

    /**
     * @brief Generate sequence diagram participant by name
     *
     * This is convienience wrapper over `generate_participant()` by id.
     *
     * @param ostr Output stream
     * @param name Full participant name
     */
    void generate_participant(
        std::ostream &ostr, const std::string &name) const;

    /**
     * @brief Generate sequence diagram activity.
     *
     * @param a Activity model
     * @param ostr Output stream
     * @param visited List of already visited participants, this is necessary
     *                for breaking infinite recursion on recursive calls
     */
    void generate_activity(const clanguml::sequence_diagram::model::activity &a,
        std::ostream &ostr, std::vector<eid_t> &visited) const;

private:
    /**
     * @brief Check if specified participant has already been generated.
     *
     * @param id Participant id.
     * @return True, if participant has already been generated.
     */
    bool is_participant_generated(eid_t id) const;

    /**
     * @brief Generate PlantUML alias for participant
     *
     * @param participant Sequence diagram participant model
     * @return Particpant alias
     */
    std::string generate_alias(const model::participant &participant) const;

    /**
     * @brief Escape the symbols in the name for PlantUML
     *
     * @param name Full participant name
     * @return Escaped name
     */
    std::string render_name(std::string name) const;

    /**
     * @brief Generate message call note
     *
     * @param ostr Output stream
     * @param m Message
     */
    void generate_message_comment(
        std::ostream &ostr, const model::message &m) const;

    /**
     * @brief Convert config to model message render mode.
     *
     * @return Method render mode.
     */
    model::function::message_render_mode
    select_method_arguments_render_mode() const;

    mutable std::set<eid_t> generated_participants_;
    mutable std::vector<model::message> already_generated_in_static_context_;
};

} // namespace plantuml
} // namespace generators
} // namespace sequence_diagram
} // namespace clanguml
