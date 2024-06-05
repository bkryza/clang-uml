/**
 * @file src/package_diagram/visitor/translation_unit_visitor.h
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

#include "common/visitor/translation_unit_visitor.h"
#include "config/config.h"
#include "package_diagram/model/diagram.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <common/model/enums.h>
#include <common/model/package.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::package_diagram::visitor {

using clanguml::common::eid_t;

using found_relationships_t =
    std::vector<std::pair<eid_t, common::model::relationship_t>>;

using visitor_specialization_t =
    common::visitor::translation_unit_visitor<clanguml::config::package_diagram,
        clanguml::package_diagram::model::diagram>;

/**
 * @brief Package diagram translation unit visitor
 *
 * This class implements the `clang::RecursiveASTVisitor` interface
 * for selected visitors relevant to generating package diagrams.
 */
class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor>,
      public visitor_specialization_t {
public:
    /**
     * @brief Constructor.
     *
     * @param sm Current source manager reference
     * @param diagram Diagram model
     * @param config Diagram configuration
     */
    translation_unit_visitor(clang::SourceManager &sm,
        clanguml::package_diagram::model::diagram &diagram,
        const clanguml::config::package_diagram &config);

    ~translation_unit_visitor() override = default;

    /**
     * \defgroup Implementation of ResursiveASTVisitor methods
     * @{
     */
    virtual bool VisitNamespaceDecl(clang::NamespaceDecl *ns);

    virtual bool VisitEnumDecl(clang::EnumDecl *decl);

    virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *cls);

    virtual bool VisitRecordDecl(clang::RecordDecl *cls);

    virtual bool VisitClassTemplateDecl(clang::ClassTemplateDecl *decl);

    virtual bool VisitFunctionDecl(clang::FunctionDecl *function_declaration);
    /** @} */

    /**
     * @brief Finalize diagram model
     */
    void finalize();

private:
    /**
     * @brief Get global package id for declaration.
     *
     * This method calculates package diagram id based on either the namespace
     * of the provided declaration or filesystem path of the source file
     * where it was declared - depending on the diagram configuration.
     *
     * This is necessary to infer dependency relationships between packages.
     *
     * @param cls C++ entity declaration
     * @return Id of the package containing that declaration
     */
    eid_t get_package_id(const clang::Decl *cls);

    /**
     * @brief Process class declaration
     *
     * @param cls Class declaration
     * @param relationships List of relationships discovered from this class
     */
    void process_class_declaration(
        const clang::CXXRecordDecl &cls, found_relationships_t &relationships);

    /**
     * @brief Process class children
     *
     * @param cls Class declaration
     * @param relationships List of relationships discovered from this class
     */
    void process_class_children(
        const clang::CXXRecordDecl &cls, found_relationships_t &relationships);

    /**
     * @brief Process record children
     *
     * @param cls Record declaration
     * @param relationships List of relationships discovered from this class
     */
    void process_record_children(
        const clang::RecordDecl &cls, found_relationships_t &relationships);

    /**
     * @brief Process record bases
     *
     * @param cls Class declaration
     * @param relationships List of relationships discovered from this class
     */
    void process_class_bases(
        const clang::CXXRecordDecl &cls, found_relationships_t &relationships);

    /**
     * @brief Process method declaration
     *
     * @param method Method declaration
     * @param relationships List of relationships discovered from this method
     */
    void process_method(const clang::CXXMethodDecl &method,
        found_relationships_t &relationships);

    /**
     * @brief Process template method declaration
     *
     * @param method Method declaration
     * @param relationships List of relationships discovered from this method
     */
    void process_template_method(const clang::FunctionTemplateDecl &method,
        found_relationships_t &relationships);

    /**
     * @brief Process member field
     *
     * @param field_declaration Field declaration
     * @param relationships List of relationships discovered from this field
     */
    void process_field(const clang::FieldDecl &field_declaration,
        found_relationships_t &relationships);

    /**
     * @brief Process static member field
     *
     * @param field_declaration Field declaration
     * @param relationships List of relationships discovered from this field
     */
    void process_static_field(const clang::VarDecl &field_declaration,
        found_relationships_t &relationships);

    /**
     * @brief Process friend declaration
     *
     * @param friend_declaration Field declaration
     * @param relationships List of relationships discovered from this friend
     */
    void process_friend(const clang::FriendDecl &friend_declaration,
        found_relationships_t &relationships);

    /**
     * @brief Process type
     *
     * @param type Reference to some C++ type
     * @param relationships List of relationships discovered from this friend
     * @param relationship_hint Default relationship type for discovered
     *                          relationships
     */
    bool find_relationships(const clang::QualType &type,
        found_relationships_t &relationships,
        common::model::relationship_t relationship_hint =
            common::model::relationship_t::kDependency);

    /**
     * @brief Add discovered relationships for `cls` to the diagram.
     *
     * @param cls C/C++ entity declaration
     * @param relationships List of discovered relationships
     */
    void add_relationships(
        clang::Decl *cls, found_relationships_t &relationships);

    std::vector<eid_t> get_parent_package_ids(eid_t id);
};
} // namespace clanguml::package_diagram::visitor
