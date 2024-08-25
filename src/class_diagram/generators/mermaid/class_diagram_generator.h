/**
 * @file src/class_diagram/generators/mermaid/class_diagram_generator.h
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

#include "class_diagram/generators/text_diagram_strategy.h"
#include "class_diagram/model/class.h"
#include "class_diagram/model/concept.h"
#include "class_diagram/model/diagram.h"
#include "class_diagram/model/enum.h"
#include "class_diagram/model/objc_interface.h"
#include "class_diagram/visitor/translation_unit_visitor.h"
#include "common/generators/mermaid/generator.h"
#include "common/generators/nested_element_stack.h"
#include "common/model/relationship.h"
#include "config/config.h"
#include "util/util.h"

#include <glob/glob.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml::class_diagram::generators::mermaid {

using diagram_config = clanguml::config::class_diagram;
using diagram_model = clanguml::class_diagram::model::diagram;
template <typename C, typename D>
using common_generator = clanguml::common::generators::mermaid::generator<C, D>;

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_element;
using clanguml::class_diagram::model::class_member;
using clanguml::class_diagram::model::class_method;
using clanguml::class_diagram::model::concept_;
using clanguml::class_diagram::model::enum_;
using clanguml::class_diagram::model::objc_interface;
using clanguml::class_diagram::model::objc_member;
using clanguml::class_diagram::model::objc_method;
using clanguml::common::model::access_t;
using clanguml::common::model::package;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;

using namespace clanguml::util;

/**
 * @brief Class diagram MermaidJS generator
 */
class generator
    : public common_generator<diagram_config, diagram_model>,
      public class_diagram::generators::text_diagram_strategy<generator> {

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
     * @brief Generate the diagram type
     *
     * @param ostr Output stream
     */
    void generate_diagram_type(std::ostream &ostr) const override;

    /**
     * @brief Generate MermaidJS alias for a class element.
     *
     * @param c Class element
     * @param ostr Output stream
     */
    void generate_alias(
        const common::model::element &e, std::ostream &ostr) const;

    /**
     * @brief Render class element to MermaidJS
     *
     * @param c Class element
     * @param ostr Output stream
     */
    void generate(const class_ &c, std::ostream &ostr) const;

    /**
     * @brief Render class methods to MermaidJS
     *
     * @param methods List of class methods
     * @param ostr Output stream
     */
    void generate_methods(
        const std::vector<class_method> &methods, std::ostream &ostr) const;

    void generate_methods(
        const std::vector<objc_method> &methods, std::ostream &ostr) const;

    void start_method_group(std::ostream &ostr) const;

    /**
     * @brief Render class method to MermaidJS
     *
     * @param m Class method
     * @param ostr Output stream
     */
    void generate_method(const class_method &m, std::ostream &ostr) const;

    void generate_method(const objc_method &m, std::ostream &ostr) const;

    /**
     * @brief Render class member to MermaidJS
     *
     * @param m Class member
     * @param ostr Output stream
     */
    void generate_member(const class_member &m, std::ostream &ostr) const;

    void generate_member(const objc_member &m, std::ostream &ostr) const;

    /**
     * @brief Render all relationships originating from class element.
     *
     * @param c Class element
     * @param ostr Output stream
     */
    void generate_relationships(const class_ &c, std::ostream &ostr) const;

    /**
     * @brief Render a specific relationship to MermaidJS.
     *
     * @param r Relationship model
     * @param rendered_relations Set of already rendered relationships, to
     *                           ensure that there are no duplicate
     *                           relationships
     */
    void generate_relationship(
        const relationship &r, std::set<std::string> &rendered_relations) const;

    /**
     * @brief Render enum element to MermaidJS
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
     * @brief Render concept element to MermaidJS
     *
     * @param c Concept element
     * @param ostr Output stream
     */
    void generate(const concept_ &c, std::ostream &ostr) const;

    /**
     * @brief Render all relationships originating from ObjC class element.
     *
     * @param c Concept element
     * @param ostr Output stream
     */
    void generate_relationships(const concept_ &c, std::ostream &ostr) const;

    void generate(const objc_interface &c, std::ostream &ostr) const;

    /**
     * @brief Render all relationships originating from ObjC class element.
     *
     * @param c Concept element
     * @param ostr Output stream
     */
    void generate_relationships(
        const objc_interface &c, std::ostream &ostr) const;

    using class_diagram::generators::text_diagram_strategy<
        generator>::generate_relationships;

    /**
     * @brief Generate any notes attached specifically to some class element.
     *
     * @param ostream Output stream
     * @param member Class element (member or method)
     * @param alias MermaidJS class alias
     */
    void generate_member_notes(std::ostream &ostream,
        const class_element &member, const std::string &alias) const;

    void start_together_group(
        const std::string &group_name, std::ostream &ostr) const;

    void end_together_group(
        const std::string &group_name, std::ostream &ostr) const;

    void start_package(const package &p, std::ostream &ostr) const;

    void end_package(const package &p, std::ostream &ostr) const;
};

} // namespace clanguml::class_diagram::generators::mermaid
