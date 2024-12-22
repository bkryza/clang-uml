/**
 * @file src/include_diagram/generators/graphml/include_diagram_generator.h
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

#include "common/generators/graphml/generator.h"
#include "common/model/package.h"
#include "common/model/relationship.h"
#include "common/model/source_file.h"
#include "config/config.h"
#include "include_diagram/model/diagram.h"
#include "include_diagram/visitor/translation_unit_visitor.h"
#include "util/util.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml::include_diagram::generators::graphml {

using diagram_config = clanguml::config::include_diagram;
using diagram_model = clanguml::include_diagram::model::diagram;

template <typename C, typename D>
using common_generator = clanguml::common::generators::graphml::generator<C, D>;

using clanguml::common::generators::graphml::graphml_node_t;
using clanguml::common::model::relationship_t;
using clanguml::common::model::source_file;
using namespace clanguml::util;

/**
 * @brief Include diagram JSON generator
 */
class generator : public common_generator<diagram_config, diagram_model> {
public:
    generator(diagram_config &config, diagram_model &model);

    using common_generator<diagram_config, diagram_model>::generate;

    std::vector<
        std::pair<std::string, common::generators::graphml::property_type>>
    node_property_names() const override;

    /**
     * @brief Generate diagram element
     *
     * @param e Source file diagram element
     * @param parent Parent XML node
     */
    void generate(const source_file &e, graphml_node_t &parent) const;

    void generate_top_level_elements(graphml_node_t &parent) const override;

private:
    void generate_with_packages(
        const source_file &f, graphml_node_t &parent) const;

    void generate_without_packages(
        const source_file &f, graphml_node_t &parent) const;
};

} // namespace clanguml::include_diagram::generators::graphml
