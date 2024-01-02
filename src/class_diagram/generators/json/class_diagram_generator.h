/**
 * @file src/class_diagram/generators/json/class_diagram_generator.h
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

#include "class_diagram/model/class.h"
#include "class_diagram/model/concept.h"
#include "class_diagram/model/diagram.h"
#include "class_diagram/model/enum.h"
#include "class_diagram/visitor/translation_unit_visitor.h"
#include "common/generators/json/generator.h"
#include "common/generators/nested_element_stack.h"
#include "common/model/relationship.h"
#include "config/config.h"
#include "util/util.h"

#include <glob/glob.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace class_diagram {
namespace generators {
namespace json {

using diagram_config = clanguml::config::class_diagram;
using diagram_model = clanguml::class_diagram::model::diagram;
template <typename C, typename D>
using common_generator = clanguml::common::generators::json::generator<C, D>;

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_element;
using clanguml::class_diagram::model::concept_;
using clanguml::class_diagram::model::enum_;
using clanguml::common::model::access_t;
using clanguml::common::model::package;
using clanguml::common::model::relationship_t;

using namespace clanguml::util;

/**
 * @brief Class diagram JSON generator
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
     * Render class element into a JSON node.
     *
     * @param c class diagram element
     * @param parent JSON node
     */
    void generate(const class_ &c, nlohmann::json &parent) const;

    /**
     * Render enum element into a JSON node.
     *
     * @param c enum diagram element
     * @param parent JSON node
     */
    void generate(const enum_ &c, nlohmann::json &parent) const;

    /**
     * Render concept element into a JSON node.
     *
     * @param c concept diagram element
     * @param parent JSON node
     */
    void generate(const concept_ &c, nlohmann::json &parent) const;

    /**
     * Render package element into a JSON node.
     *
     * @param p package diagram element
     * @param parent JSON node
     */
    void generate(const package &p, nlohmann::json &parent) const;

    /**
     * @brief In a nested diagram, generate the top level elements.
     *
     * This method iterates over the top level elements. In case the diagram
     * is nested (i.e. includes packages), for each package it recursively
     * call generation of elements contained in each package.
     *
     * @param parent JSON node
     */
    void generate_top_level_elements(nlohmann::json &parent) const;

    /**
     * @brief Generate all relationships in the diagram.
     *
     * @param parent JSON node
     */
    void generate_relationships(nlohmann::json &parent) const;

    /**
     * @brief Generate all relationships originating at a class element.
     *
     * @param c Class diagram element
     * @param parent JSON node
     */
    void generate_relationships(const class_ &c, nlohmann::json &parent) const;

    /**
     * @brief Generate all relationships originating at an enum element.
     *
     * @param c Enum diagram element
     * @param parent JSON node
     */
    void generate_relationships(const enum_ &c, nlohmann::json &parent) const;

    /**
     * @brief Generate all relationships originating at a concept element.
     *
     * @param c Concept diagram element
     * @param parent JSON node
     */
    void generate_relationships(
        const concept_ &c, nlohmann::json &parent) const;

    /**
     * @brief Generate all relationships in a package.
     *
     * If the diagram is nested, it recursively calls relationship generation
     * for all subelements.
     *
     * @param p Package diagram element
     * @param parent JSON node
     */
    void generate_relationships(const package &p, nlohmann::json &parent) const;
};

} // namespace json
} // namespace generators
} // namespace class_diagram
} // namespace clanguml
