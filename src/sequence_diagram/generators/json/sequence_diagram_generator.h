/**
 * src/sequence_diagram/generators/json/sequence_diagram_generator.h
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

using diagram_config = clanguml::config::sequence_diagram;
using diagram_model = clanguml::sequence_diagram::model::diagram;

template <typename C, typename D>
using common_generator = clanguml::common::generators::json::generator<C, D>;

class generator : public common_generator<diagram_config, diagram_model> {
public:
    generator(diagram_config &config, diagram_model &model);

    void generate_call(const sequence_diagram::model::message &m,
        nlohmann::json &parent) const;

    common::id_t generate_participant(
        nlohmann::json &parent, common::id_t id, bool force = false) const;

    void generate_participant(
        nlohmann::json &parent, const std::string &name) const;

    void generate_activity(common::model::diagram_element::id_t activity_id,
        const sequence_diagram::model::activity &a, nlohmann::json &parent,
        std::vector<common::model::diagram_element::id_t> &visited,
        std::optional<nlohmann::json> nested_block) const;

    void generate(std::ostream &ostr) const override;

    nlohmann::json &current_block_statement() const
    {
        assert(!block_statements_stack_.empty());

        return block_statements_stack_.back().get();
    }

private:
    bool is_participant_generated(common::id_t id) const;

    std::string render_name(std::string name) const;

    mutable std::set<common::id_t> generated_participants_;

    mutable nlohmann::json json_;

    mutable std::vector<std::reference_wrapper<nlohmann::json>>
        block_statements_stack_;
};

} // namespace clanguml::sequence_diagram::generators::json
