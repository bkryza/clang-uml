/**
 * @file src/class_diagram/generators/graphml/class_diagram_generator.h
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
#include "common/generators/graphml/generator.h"
#include "common/generators/nested_element_stack.h"
#include "common/model/relationship.h"
#include "config/config.h"
#include "util/util.h"

#include <glob/glob.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace class_diagram {
namespace generators {
namespace graphml {

using diagram_config = clanguml::config::class_diagram;
using diagram_model = clanguml::class_diagram::model::diagram;
template <typename C, typename D>
using common_generator = clanguml::common::generators::graphml::generator<C, D>;

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_element;
using clanguml::class_diagram::model::concept_;
using clanguml::class_diagram::model::enum_;
using clanguml::class_diagram::model::objc_interface;
using clanguml::common::model::access_t;
using clanguml::common::model::package;
using clanguml::common::model::relationship_t;

using clanguml::common::generators::graphml::graph_t;

using namespace clanguml::util;

/**
 * @brief Class diagram GraphML generator
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
    void generate_diagram(graph_t &parent) const override;

    /**
     * Render class element into a GraphML node.
     *
     * @param c class diagram element
     * @param parent GraphML node
     */
    void generate(const class_ &c, graph_t &parent) const;

    /**
     * Render ObjC interface or protocol element into a GraphML node.
     *
     * @param c enum diagram element
     * @param parent GraphML node
     */
    void generate(const objc_interface &c, graph_t &parent) const;

    /**
     * Render enum element into a GraphML node.
     *
     * @param c enum diagram element
     * @param parent GraphML node
     */
    void generate(const enum_ &c, graph_t &parent) const;

    /**
     * Render concept element into a GraphML node.
     *
     * @param c concept diagram element
     * @param parent JSON node
     */
    void generate(const concept_ &c, graph_t &parent) const;

    /**
     * Render package element into a GraphML node.
     *
     * @param p package diagram element
     * @param parent GraphML node
     */
    void generate(const package &p, graph_t &parent) const;

    /**
     * @brief In a nested diagram, generate the top level elements.
     *
     * This method iterates over the top level elements. In case the diagram
     * is nested (i.e. includes packages), for each package it recursively
     * call generation of elements contained in each package.
     *
     * @param parent GraphML node
     */
    void generate_top_level_elements(graph_t &parent) const;

    /**
     * @brief Generate all relationships in the diagram.
     *
     * @param parent GraphML node
     */
    void generate_relationships(graph_t &parent) const;

    /**
     * @brief Generate all relationships originating at a diagram element.
     *
     * @tparam T Type of diagram element
     * @param c Diagram diagram element
     * @param parent JSON node
     */
    template <typename T>
    void generate_relationships(const T &c, graph_t &parent) const;
};

template <typename T>
void generator::generate_relationships(const T &c, graph_t &parent) const
{
    const auto &model =
        common_generator<diagram_config, diagram_model>::model();

    for (const auto &r : c.relationships()) {
        auto target_element = model.get(r.destination());
        if (!target_element.has_value()) {
            LOG_DBG("Skipping {} relation from '{}' to '{}' due "
                    "to unresolved destination id",
                to_string(r.type()), c.full_name(true),
                r.destination().value());
            continue;
        }

        const auto target_id = target_element.value().id().value();
        const auto src_id = c.id().value();

        if (parent.vertex(target_id) == graph_t::null_vertex())
            continue;

        if (parent.vertex(src_id) == graph_t::null_vertex())
            continue;

        boost::add_edge_by_label(src_id, target_id, parent);

        //        nlohmann::json rel = r;
        //        rel["source"] = std::to_string(c.id().value());
        //        parent["relationships"].push_back(rel);
    }
}

template <>
void generator::generate_relationships<package>(
    const package &p, graph_t &parent) const;

} // namespace json
} // namespace generators
} // namespace class_diagram
} // namespace clanguml
