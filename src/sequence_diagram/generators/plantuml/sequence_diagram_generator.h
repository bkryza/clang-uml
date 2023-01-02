/**
 * src/sequence_diagram/generators/plantuml/sequence_diagram_generator.h
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

using diagram_config = clanguml::config::sequence_diagram;
using diagram_model = clanguml::sequence_diagram::model::diagram;

template <typename C, typename D>
using common_generator =
    clanguml::common::generators::plantuml::generator<C, D>;

class generator : public common_generator<diagram_config, diagram_model> {
public:
    generator(diagram_config &config, diagram_model &model);

    void generate_call(const clanguml::sequence_diagram::model::message &m,
        std::ostream &ostr) const;

    void generate_return(const clanguml::sequence_diagram::model::message &m,
        std::ostream &ostr) const;

    void generate_participant(
        std::ostream &ostr, common::id_t id, bool force = false) const;

    void generate_participant(
        std::ostream &ostr, const std::string &name) const;

    void generate_activity(const clanguml::sequence_diagram::model::activity &a,
        std::ostream &ostr,
        std::vector<common::model::diagram_element::id_t> &visited) const;

    void generate(std::ostream &ostr) const override;

private:
    bool is_participant_generated(common::id_t id) const;

    std::string render_name(std::string name) const;

    mutable std::set<common::id_t> generated_participants_;
    std::string generate_alias(const model::participant &participant) const;
};

} // namespace plantuml
} // namespace generators
} // namespace sequence_diagram
} // namespace clanguml
