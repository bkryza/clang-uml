/**
 * src/class_diagram/visitor/translation_unit_visitor.h
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

#include "class_diagram/model/class.h"
#include "class_diagram/model/concept.h"
#include "class_diagram/model/diagram.h"
#include "class_diagram/visitor/template_builder.h"
#include "common/model/enums.h"
#include "common/model/template_trait.h"
#include "common/visitor/ast_id_mapper.h"
#include "common/visitor/translation_unit_visitor.h"
#include "config/config.h"
#include "template_builder.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::class_diagram::visitor {

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_member;
using clanguml::class_diagram::model::class_method;
using clanguml::class_diagram::model::class_parent;
using clanguml::class_diagram::model::concept_;
using clanguml::class_diagram::model::diagram;
using clanguml::class_diagram::model::enum_;
using clanguml::class_diagram::model::method_parameter;
using clanguml::common::model::access_t;
using clanguml::common::model::namespace_;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;
using clanguml::common::model::template_parameter;
using clanguml::common::model::template_trait;

/**
 * @brief Class diagram translation unit visitor
 *
 * This class implements the @link clang::RecursiveASTVisitor interface
 * for selected visitors relevant to generating class diagrams.
 */
class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor>,
      public common::visitor::translation_unit_visitor {
public:
    explicit translation_unit_visitor(clang::SourceManager &sm,
        clanguml::class_diagram::model::diagram &diagram,
        const clanguml::config::class_diagram &config);

    bool shouldVisitTemplateInstantiations() const { return false; }

    bool shouldVisitImplicitCode() const { return false; }

    virtual bool VisitNamespaceDecl(clang::NamespaceDecl *ns);

    virtual bool VisitRecordDecl(clang::RecordDecl *D);

    virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *d);

    virtual bool VisitEnumDecl(clang::EnumDecl *e);

    virtual bool VisitClassTemplateDecl(
        clang::ClassTemplateDecl *class_template_declaration);

    virtual bool VisitClassTemplateSpecializationDecl(
        clang::ClassTemplateSpecializationDecl *cls);

    virtual bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *cls);

    virtual bool TraverseConceptDecl(clang::ConceptDecl *cpt);

    /**
     * @brief Get diagram model reference
     *
     * @return Reference to diagram model created by the visitor
     */
    clanguml::class_diagram::model::diagram &diagram() { return diagram_; }

    const clanguml::class_diagram::model::diagram &diagram() const
    {
        return diagram_;
    }

    /**
     * @brief Get diagram config instance
     *
     * @return Reference to config instance
     */
    const clanguml::config::class_diagram &config() const { return config_; }

    /**
     * @brief Finalize diagram model
     *
     * This method is called after the entire AST has been visited by this
     * visitor. It is used to perform necessary post processing on the diagram
     * (e.g. resolve translation unit local element ID's into global ID's based
     * on elements full names).
     */
    void finalize();

    common::visitor::ast_id_mapper &id_mapper() const { return id_mapper_; }

    void add_class(std::unique_ptr<class_> &&c);
    void add_enum(std::unique_ptr<enum_> &&e);
    void add_concept(std::unique_ptr<concept_> &&c);

private:
    bool should_include(const clang::NamedDecl *decl);

    std::unique_ptr<clanguml::class_diagram::model::class_>
    create_class_declaration(clang::CXXRecordDecl *cls);

    std::unique_ptr<clanguml::class_diagram::model::class_>
    create_record_declaration(clang::RecordDecl *rec);

    std::unique_ptr<clanguml::class_diagram::model::concept_>
    create_concept_declaration(clang::ConceptDecl *cpt);

    void process_class_declaration(const clang::CXXRecordDecl &cls,
        clanguml::class_diagram::model::class_ &c);

    void process_class_bases(const clang::CXXRecordDecl *cls,
        clanguml::class_diagram::model::class_ &c);

    void process_class_children(const clang::CXXRecordDecl *cls,
        clanguml::class_diagram::model::class_ &c);

    void process_record_members(const clang::RecordDecl *cls, class_ &c);

    std::unique_ptr<clanguml::class_diagram::model::class_>
    process_template_specialization(
        clang::ClassTemplateSpecializationDecl *cls);

    void process_template_specialization_children(
        const clang::ClassTemplateSpecializationDecl *cls, class_ &c);

    bool process_template_parameters(
        const clang::TemplateDecl &template_declaration,
        clanguml::common::model::template_trait &t,
        common::optional_ref<common::model::element> templated_element = {});

    void process_method(const clang::CXXMethodDecl &mf,
        clanguml::class_diagram::model::class_ &c);

    void process_template_method(const clang::FunctionTemplateDecl &mf,
        clanguml::class_diagram::model::class_ &c);

    void process_static_field(const clang::VarDecl &field_declaration,
        clanguml::class_diagram::model::class_ &c);

    void process_field(const clang::FieldDecl &field_declaration,
        clanguml::class_diagram::model::class_ &c);

    void process_function_parameter(const clang::ParmVarDecl &param,
        clanguml::class_diagram::model::class_method &method,
        clanguml::class_diagram::model::class_ &c,
        const std::set<std::string> &template_parameter_names = {});

    void process_friend(
        const clang::FriendDecl &f, clanguml::class_diagram::model::class_ &c);

    bool find_relationships(const clang::QualType &type,
        found_relationships_t & /*relationships*/,
        clanguml::common::model::relationship_t relationship_hint);

    void add_relationships(clanguml::class_diagram::model::class_ &c,
        const clanguml::class_diagram::model::class_member &field,
        const found_relationships_t &relationships,
        bool break_on_first_aggregation = false);

    void ensure_lambda_type_is_relative(std::string &parameter_type) const;

    void process_record_parent(
        clang::RecordDecl *cls, class_ &c, const namespace_ &ns);

    void process_function_parameter_find_relationships_in_autotype(
        model::class_ &c, const clang::AutoType *atsp);

    void find_relationships_in_constraint_expression(
        clanguml::common::model::element &c, const clang::Expr *expr);

    void add_incomplete_forward_declarations();

    void resolve_local_to_global_ids();

    void process_constraint_requirements(const clang::ConceptDecl *cpt,
        const clang::Expr *expr, model::concept_ &concept_model) const;

    void process_concept_specialization_relationships(common::model::element &c,
        const clang::ConceptSpecializationExpr *concept_specialization);

    void extract_constrained_template_param_name(
        const clang::ConceptSpecializationExpr *concept_specialization,
        const clang::ConceptDecl *cpt,
        std::vector<std::string> &constrained_template_params,
        size_t argument_index, std::string &type_name) const;

    /// Store the mapping from local clang entity id (obtained using
    /// getID()) method to clang-uml global id
    void set_ast_local_id(
        int64_t local_id, common::model::diagram_element::id_t global_id);

    void add_processed_template_class(std::string qualified_name);

    bool has_processed_template_class(const std::string &qualified_name) const;

    template_builder &tbuilder() { return template_builder_; }

    // Reference to the output diagram model
    clanguml::class_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::class_diagram &config_;

    mutable common::visitor::ast_id_mapper id_mapper_;

    template_builder template_builder_;

    std::map<common::model::diagram_element::id_t,
        std::unique_ptr<clanguml::class_diagram::model::class_>>
        forward_declarations_;

    std::map<int64_t /* local anonymous struct id */,
        std::tuple<std::string /* field name */, common::model::relationship_t,
            common::model::access_t>>
        anonymous_struct_relationships_;

    // When visiting CXX records we need to know if they have already been
    // process in VisitClassTemplateDecl or VisitClassTemplateSpecializationDecl
    // If yes, then we need to skip it
    // TODO: There must be a better way to do this...
    std::set<std::string> processed_template_qualified_names_;
};
} // namespace clanguml::class_diagram::visitor
