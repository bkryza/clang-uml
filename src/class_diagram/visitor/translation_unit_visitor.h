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
#include "class_diagram/model/diagram.h"
#include "common/model/enums.h"
#include "common/model/template_trait.h"
#include "common/visitor/translation_unit_visitor.h"
#include "config/config.h"

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
using clanguml::class_diagram::model::diagram;
using clanguml::class_diagram::model::enum_;
using clanguml::class_diagram::model::method_parameter;
using clanguml::common::model::access_t;
using clanguml::common::model::namespace_;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;
using clanguml::common::model::template_parameter;
using clanguml::common::model::template_trait;

using found_relationships_t =
    std::vector<std::pair<clanguml::common::model::diagram_element::id_t,
        common::model::relationship_t>>;

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

    virtual bool VisitNamespaceDecl(clang::NamespaceDecl *ns);

    virtual bool VisitRecordDecl(clang::RecordDecl *D);

    virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *d);

    virtual bool VisitEnumDecl(clang::EnumDecl *e);

    virtual bool VisitClassTemplateDecl(
        clang::ClassTemplateDecl *class_template_declaration);

    virtual bool VisitClassTemplateSpecializationDecl(
        clang::ClassTemplateSpecializationDecl *cls);

    virtual bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *cls);

    /**
     * @brief Get diagram model reference
     *
     * @return Reference to diagram model created by the visitor
     */
    clanguml::class_diagram::model::diagram &diagram() { return diagram_; }

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

private:
    std::unique_ptr<clanguml::class_diagram::model::class_>
    create_class_declaration(clang::CXXRecordDecl *cls);

    std::unique_ptr<clanguml::class_diagram::model::class_>
    create_record_declaration(clang::RecordDecl *rec);

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
        clanguml::common::model::template_trait &t);

    void process_template_specialization_argument(
        const clang::ClassTemplateSpecializationDecl *cls,
        model::class_ &template_instantiation,
        const clang::TemplateArgument &arg, size_t argument_index,
        bool in_parameter_pack = false);

    void process_template_record_containment(const clang::TagDecl &record,
        clanguml::common::model::element &c) const;

    void process_record_containment(const clang::TagDecl &record,
        clanguml::common::model::element &c) const;

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

    std::unique_ptr<clanguml::class_diagram::model::class_>
    build_template_instantiation(
        const clang::TemplateSpecializationType &template_type,
        std::optional<clanguml::class_diagram::model::class_ *> parent = {});

    std::unique_ptr<clanguml::class_diagram::model::class_>
    build_template_instantiation_from_class_template_specialization(
        const clang::ClassTemplateSpecializationDecl &template_specialization,
        const clang::RecordType &record_type,
        std::optional<clanguml::class_diagram::model::class_ *> parent = {});

    bool build_template_instantiation_add_base_classes(
        clanguml::class_diagram::model::class_ &tinst,
        std::deque<std::tuple<std::string, int, bool>> &template_base_params,
        int arg_index, bool variadic_params,
        const clanguml::common::model::template_parameter &ct) const;

    void build_template_instantiation_process_template_arguments(
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        std::deque<std::tuple<std::string, int, bool>> &template_base_params,
        const clang::ArrayRef<clang::TemplateArgument> &template_args,
        model::class_ &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl);

    void build_template_instantiation_process_tag_argument(
        model::class_ &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        common::model::template_parameter &argument);

    void build_template_instantiation_process_expression_argument(
        const clang::TemplateArgument &arg,
        common::model::template_parameter &argument) const;

    void build_template_instantiation_process_integral_argument(
        const clang::TemplateArgument &arg,
        common::model::template_parameter &argument) const;

    void build_template_instantiation_process_type_argument(
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        model::class_ &template_instantiation,
        common::model::template_parameter &argument);

    void build_template_instantiation_process_template_argument(
        const clang::TemplateArgument &arg,
        common::model::template_parameter &argument) const;

    void ensure_lambda_type_is_relative(std::string &parameter_type) const;

    void process_record_parent(
        clang::RecordDecl *cls, class_ &c, const namespace_ &ns);

    void process_function_parameter_find_relatinoships_in_autotype(
        model::class_ &c, const clang::AutoType *atsp);

    void process_function_parameter_find_relationships_in_template(
        clanguml::class_diagram::model::class_ &c,
        const std::set<std::string> &template_parameter_names,
        const clang::TemplateSpecializationType &template_instantiation_type);

    void process_unexposed_template_specialization_parameters(
        const std::string &tspec,
        clanguml::common::model::template_parameter &tp,
        clanguml::class_diagram::model::class_ &c);

    bool find_relationships_in_unexposed_template_params(
        const clanguml::common::model::template_parameter &ct,
        found_relationships_t &relationships);

    void add_incomplete_forward_declarations();

    void resolve_local_to_global_ids();

    bool simplify_system_template(common::model::template_parameter &ct,
        const std::string &full_name) const;

    /// Store the mapping from local clang entity id (obtained using
    /// getID()) method to clang-uml global id
    void set_ast_local_id(
        int64_t local_id, common::model::diagram_element::id_t global_id);

    /// Retrieve the global clang-uml entity id based on the clang local id
    std::optional<common::model::diagram_element::id_t> get_ast_local_id(
        int64_t local_id) const;

    // Reference to the output diagram model
    clanguml::class_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::class_diagram &config_;

    std::map<common::model::diagram_element::id_t,
        std::unique_ptr<clanguml::class_diagram::model::class_>>
        forward_declarations_;

    std::map<int64_t, common::model::diagram_element::id_t> local_ast_id_map_;

    std::map<int64_t /* local anonymous struct id */,
        std::tuple<std::string /* field name */, common::model::relationship_t,
            common::model::access_t>>
        anonymous_struct_relationships_;
};
} // namespace clanguml::class_diagram::visitor
