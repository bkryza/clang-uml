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

namespace clanguml::class_diagram::generators::graphml {

using diagram_config = clanguml::config::class_diagram;
using diagram_model = clanguml::class_diagram::model::diagram;
template <typename C, typename D>
using common_generator = clanguml::common::generators::graphml::generator<C, D>;

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_element;
using clanguml::class_diagram::model::concept_;
using clanguml::class_diagram::model::enum_;
using clanguml::class_diagram::model::objc_interface;
using clanguml::common::generators::graphml::graphml_node_t;
using clanguml::common::model::access_t;
using clanguml::common::model::package;
using clanguml::common::model::relationship_t;

using namespace clanguml::util;

/**
 * @brief Class diagram GraphML generator
 */
class generator : public common_generator<diagram_config, diagram_model> {
public:
    generator(diagram_config &config, diagram_model &model);

    using common_generator<diagram_config, diagram_model>::generate;

    std::vector<
        std::pair<std::string, common::generators::graphml::property_type>>
    node_property_names() const override;

    /**
     * Render class element into a GraphML node.
     *
     * @param c class diagram element
     * @param parent GraphML node
     */
    void generate(const class_ &c, graphml_node_t &parent) const;

    /**
     * Render ObjC interface or protocol element into a GraphML node.
     *
     * @param c enum diagram element
     * @param parent GraphML node
     */
    void generate(const objc_interface &c, graphml_node_t &parent) const;

    /**
     * Render enum element into a GraphML node.
     *
     * @param c enum diagram element
     * @param parent GraphML node
     */
    void generate(const enum_ &c, graphml_node_t &parent) const;

    /**
     * Render concept element into a GraphML node.
     *
     * @param c concept diagram element
     * @param parent JSON node
     */
    void generate(const concept_ &c, graphml_node_t &parent) const;

    /**
     * Render package element into a GraphML node.
     *
     * @param p package diagram element
     * @param parent GraphML node
     */
    void generate(const package &p, graphml_node_t &parent) const override;

    void generate_top_level_elements(graphml_node_t &parent) const override;
};

} // namespace clanguml::class_diagram::generators::graphml
