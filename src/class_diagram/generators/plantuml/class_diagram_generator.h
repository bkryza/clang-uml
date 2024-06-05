/**
 * @file src/class_diagram/generators/plantuml/class_diagram_generator.h
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
#include "common/generators/nested_element_stack.h"
#include "common/generators/plantuml/generator.h"
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
namespace plantuml {

using diagram_config = clanguml::config::class_diagram;
using diagram_model = clanguml::class_diagram::model::diagram;
template <typename C, typename D>
using common_generator =
    clanguml::common::generators::plantuml::generator<C, D>;

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_element;
using clanguml::class_diagram::model::class_member;
using clanguml::class_diagram::model::class_method;
using clanguml::class_diagram::model::concept_;
using clanguml::class_diagram::model::enum_;
using clanguml::common::eid_t;
using clanguml::common::model::access_t;
using clanguml::common::model::package;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;

using namespace clanguml::util;

/**
 * @brief Class diagram PlantUML generator
 */
class generator : public common_generator<diagram_config, diagram_model> {
    using method_groups_t = std::map<std::string, std::vector<class_method>>;

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
     * @brief In a nested diagram, generate the top level elements.
     *
     * This method iterates over the top level elements. In case the diagram
     * is nested (i.e. includes packages), for each package it recursively
     * call generation of elements contained in each package.
     *
     * @param parent JSON node
     */
    void generate_top_level_elements(std::ostream &ostr) const;

    /**
     * @brief Generate a hyperlink for a class element.
     *
     * @param ostr Output stream
     * @param e Class element (e.g. a method)
     */
    void generate_link(std::ostream &ostr, const class_element &e) const;

    /**
     * @brief Generate PlantUML alias for a class element.
     *
     * @param c Class element
     * @param ostr Output stream
     */
    void generate_alias(const class_ &c, std::ostream &ostr) const;

    /**
     * @brief Generate PlantUML alias for a enum element.
     *
     * @param e Enum element
     * @param ostr Output stream
     */
    void generate_alias(const enum_ &e, std::ostream &ostr) const;

    /**
     * @brief Generate PlantUML alias for a concept element.
     *
     * @param c Concept element
     * @param ostr Output stream
     */
    void generate_alias(const concept_ &c, std::ostream &ostr) const;

    /**
     * @brief Render class element to PlantUML
     *
     * @param c Class element
     * @param ostr Output stream
     */
    void generate(const class_ &c, std::ostream &ostr) const;

    /**
     * @brief Render class methods to PlantUML
     *
     * @param methods List of class methods
     * @param ostr Output stream
     */
    void generate_methods(
        const std::vector<class_method> &methods, std::ostream &ostr) const;

    /**
     * @brief Render class methods to PlantUML in groups
     *
     * @param methods Methods grouped by method type
     * @param ostr Output stream
     */
    void generate_methods(
        const method_groups_t &methods, std::ostream &ostr) const;

    /**
     * @brief Render class method to PlantUML
     *
     * @param m Class method
     * @param ostr Output stream
     */
    void generate_method(const class_method &m, std::ostream &ostr) const;

    /**
     * @brief Render class member to PlantUML
     *
     * @param m Class member
     * @param ostr Output stream
     */
    void generate_member(const class_member &m, std::ostream &ostr) const;

    /**
     * @brief Render all relationships in the diagram to PlantUML
     *
     * @param ostr Output stream
     */
    void generate_relationships(std::ostream &ostr) const;

    /**
     * @brief Render all relationships originating from class element.
     *
     * @param c Class element
     * @param ostr Output stream
     */
    void generate_relationships(const class_ &c, std::ostream &ostr) const;

    /**
     * @brief Render a specific relationship to PlantUML.
     *
     * @param r Relationship model
     * @param rendered_relations Set of already rendered relationships, to
     *                           ensure that there are no duplicate
     *                           relationships
     */
    void generate_relationship(
        const relationship &r, std::set<std::string> &rendered_relations) const;

    /**
     * @brief Render enum element to PlantUML
     *
     * @param e Enum element
     * @param ostr Output stream
     */
    void generate(const enum_ &e, std::ostream &ostr) const;

    /**
     * @brief Render all relationships originating from enum element.
     *
     * @param c Enum element
     * @param ostr Output stream
     */
    void generate_relationships(const enum_ &c, std::ostream &ostr) const;

    /**
     * @brief Render concept element to PlantUML
     *
     * @param c Concept element
     * @param ostr Output stream
     */
    void generate(const concept_ &c, std::ostream &ostr) const;

    /**
     * @brief Render all relationships originating from concept element.
     *
     * @param c Concept element
     * @param ostr Output stream
     */
    void generate_relationships(const concept_ &c, std::ostream &ostr) const;

    /**
     * @brief Render package element to PlantUML
     *
     * @param p Package element
     * @param ostr Output stream
     */
    void generate(const package &p, std::ostream &ostr) const;

    /**
     * @brief Render all relationships originating from package element.
     *
     * @param p Package element
     * @param ostr Output stream
     */
    void generate_relationships(const package &p, std::ostream &ostr) const;

    /**
     * @brief Generate any notes attached specifically to some class element.
     *
     * @param ostream Output stream
     * @param member Class element (member or method)
     * @param alias PlantUML class alias
     */
    void generate_member_notes(std::ostream &ostream,
        const class_element &member, const std::string &alias) const;

    /**
     * @brief Generate elements grouped together in `together` groups.
     *
     * @param ostr Output stream
     */
    void generate_groups(std::ostream &ostr) const;

    /**
     * @brief Group class methods based on method type.
     *
     * @param methods List of class methods.
     *
     * @return Map of method groups.
     */
    method_groups_t group_methods(
        const std::vector<class_method> &methods) const;

private:
    const std::vector<std::string> method_groups_{
        "constructors", "assignment", "operators", "other"};

    std::string render_name(std::string name) const;

    template <typename T>
    void sort_class_elements(std::vector<T> &elements) const
    {
        if (config().member_order() == config::member_order_t::lexical) {
            std::sort(elements.begin(), elements.end(),
                [](const auto &m1, const auto &m2) {
                    return m1.name() < m2.name();
                });
        }
    }

    mutable common::generators::nested_element_stack<common::model::element>
        together_group_stack_;
};

} // namespace plantuml
} // namespace generators
} // namespace class_diagram
} // namespace clanguml
