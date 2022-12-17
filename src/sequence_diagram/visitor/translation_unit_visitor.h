/**
 * src/sequence_diagram/visitor/translation_unit_visitor.h
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

#include "call_expression_context.h"
#include "common/visitor/translation_unit_visitor.h"
#include "config/config.h"
#include "sequence_diagram/model/diagram.h"

#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <stack>

namespace clanguml::sequence_diagram::visitor {

std::string to_string(const clang::FunctionTemplateDecl *decl);

class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor>,
      public common::visitor::translation_unit_visitor {
public:
    translation_unit_visitor(clang::SourceManager &sm,
        clanguml::sequence_diagram::model::diagram &diagram,
        const clanguml::config::sequence_diagram &config);

    bool shouldVisitTemplateInstantiations();

    bool VisitCallExpr(clang::CallExpr *expr);

    bool TraverseCallExpr(clang::CallExpr *expr);

    bool TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr *expr);

    bool TraverseCXXOperatorCallExpr(clang::CXXOperatorCallExpr *expr);

    // TODO
    // bool TraverseCXXConstructExpr(clang::CXXConstructExpr *expr);

    bool VisitLambdaExpr(clang::LambdaExpr *expr);

    bool TraverseLambdaExpr(clang::LambdaExpr *expr);

    bool VisitCXXMethodDecl(clang::CXXMethodDecl *method);

    bool VisitCXXRecordDecl(clang::CXXRecordDecl *cls);

    bool VisitClassTemplateDecl(clang::ClassTemplateDecl *cls);

    bool VisitClassTemplateSpecializationDecl(
        clang::ClassTemplateSpecializationDecl *cls);

    bool VisitFunctionDecl(clang::FunctionDecl *function_declaration);

    bool VisitFunctionTemplateDecl(
        clang::FunctionTemplateDecl *function_declaration);

    bool TraverseCompoundStmt(clang::CompoundStmt *stmt);

    bool TraverseIfStmt(clang::IfStmt *stmt);

    bool TraverseWhileStmt(clang::WhileStmt *stmt);

    bool TraverseDoStmt(clang::DoStmt *stmt);

    bool TraverseForStmt(clang::ForStmt *stmt);

    bool TraverseCXXForRangeStmt(clang::CXXForRangeStmt *stmt);

    bool TraverseCXXTryStmt(clang::CXXTryStmt *stmt);

    bool TraverseCXXCatchStmt(clang::CXXCatchStmt *stmt);

    bool TraverseSwitchStmt(clang::SwitchStmt *stmt);

    bool TraverseCaseStmt(clang::CaseStmt *stmt);

    bool TraverseDefaultStmt(clang::DefaultStmt *stmt);

    bool TraverseConditionalOperator(clang::ConditionalOperator *stmt);

    clanguml::sequence_diagram::model::diagram &diagram();

    const clanguml::sequence_diagram::model::diagram &diagram() const;

    const clanguml::config::sequence_diagram &config() const;

    call_expression_context &context();

    const call_expression_context &context() const;

    void finalize();

    template <typename T = model::participant>
    common::optional_ref<T> get_participant(const clang::Decl *decl)
    {
        assert(decl != nullptr);

        auto unique_participant_id = get_unique_id(decl->getID());
        if (!unique_participant_id.has_value())
            return {};

        return get_participant<T>(unique_participant_id.value());
    }

    template <typename T = model::participant>
    const common::optional_ref<T> get_participant(const clang::Decl *decl) const
    {
        assert(decl != nullptr);

        auto unique_participant_id = get_unique_id(decl->getID());
        if (!unique_participant_id.has_value())
            return {};

        return get_participant<T>(unique_participant_id.value());
    }

    template <typename T = model::participant>
    common::optional_ref<T> get_participant(
        const common::model::diagram_element::id_t id)
    {
        if (diagram().participants().find(id) == diagram().participants().end())
            return {};

        return common::optional_ref<T>(
            *(static_cast<T *>(diagram().participants().at(id).get())));
    }

    template <typename T = model::participant>
    const common::optional_ref<T> get_participant(
        const common::model::diagram_element::id_t id) const
    {
        if (diagram().participants().find(id) == diagram().participants().end())
            return {};

        return common::optional_ref<T>(
            *(static_cast<T *>(diagram().participants().at(id).get())));
    }

    /// Store the mapping from local clang entity id (obtained using
    /// getID()) method to clang-uml global id
    void set_unique_id(
        int64_t local_id, common::model::diagram_element::id_t global_id);

    /// Retrieve the global clang-uml entity id based on the clang local id
    std::optional<common::model::diagram_element::id_t> get_unique_id(
        int64_t local_id) const;

private:
    std::unique_ptr<clanguml::sequence_diagram::model::class_>
    create_class_declaration(clang::CXXRecordDecl *cls);

    bool process_template_parameters(
        const clang::TemplateDecl &template_declaration,
        sequence_diagram::model::template_trait &c);

    std::unique_ptr<model::function_template>
    build_function_template_instantiation(const clang::FunctionDecl &pDecl);

    void build_template_instantiation_process_template_arguments(
        model::template_trait *parent,
        std::deque<std::tuple<std::string, int, bool>> &template_base_params,
        const clang::ArrayRef<clang::TemplateArgument> &template_args,
        model::template_trait &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl);

    void build_template_instantiation_process_template_argument(
        const clang::TemplateArgument &arg,
        class_diagram::model::template_parameter &argument) const;

    void build_template_instantiation_process_integral_argument(
        const clang::TemplateArgument &arg,
        class_diagram::model::template_parameter &argument) const;

    void build_template_instantiation_process_expression_argument(
        const clang::TemplateArgument &arg,
        class_diagram::model::template_parameter &argument) const;

    void build_template_instantiation_process_tag_argument(
        model::template_trait &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        class_diagram::model::template_parameter &argument) const;

    void build_template_instantiation_process_type_argument(
        model::template_trait *parent,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        model::template_trait &template_instantiation,
        class_diagram::model::template_parameter &argument);

    std::unique_ptr<model::class_> process_template_specialization(
        clang::ClassTemplateSpecializationDecl *cls);

    void process_template_specialization_argument(
        const clang::ClassTemplateSpecializationDecl *cls,
        model::class_ &template_instantiation,
        const clang::TemplateArgument &arg, size_t argument_index,
        bool in_parameter_pack = false);

    void process_unexposed_template_specialization_parameters(
        const std::string &type_name,
        class_diagram::model::template_parameter &tp, model::class_ &c) const;

    std::unique_ptr<model::class_> build_template_instantiation(
        const clang::TemplateSpecializationType &template_type_decl,
        model::class_ *parent);

    bool simplify_system_template(class_diagram::model::template_parameter &ct,
        const std::string &full_name);

    std::string simplify_system_template(const std::string &full_name) const;

    std::string make_lambda_name(const clang::CXXRecordDecl *cls) const;

    bool is_smart_pointer(const clang::TemplateDecl *primary_template) const;

    bool is_callee_valid_template_specialization(
        const clang::CXXDependentScopeMemberExpr *dependent_member_expr) const;

    bool process_operator_call_expression(model::message &m,
        const clang::CXXOperatorCallExpr *operator_call_expr);

    bool process_class_method_call_expression(
        model::message &m, const clang::CXXMemberCallExpr *operator_call_expr);

    bool process_class_template_method_call_expression(
        model::message &m, const clang::CallExpr *expr);

    bool process_function_call_expression(
        model::message &m, const clang::CallExpr *expr);

    bool process_unresolved_lookup_call_expression(
        model::message &m, const clang::CallExpr *expr);

    void push_message(clang::CallExpr *expr, model::message &&m);

    void pop_message_to_diagram(clang::CallExpr *expr);

    // Reference to the output diagram model
    clanguml::sequence_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::sequence_diagram &config_;

    call_expression_context call_expression_context_;

    /// This is used to generate messages in proper order in case of
    /// nested call expressions (e.g. a(b(c(), d())), as they need to
    /// be added to the diagram sequence after the visitor leaves the
    /// call expression AST node
    std::map<clang::CallExpr *, model::message> call_expr_message_map_;

    std::map<common::model::diagram_element::id_t,
        std::unique_ptr<clanguml::sequence_diagram::model::class_>>
        forward_declarations_;

    std::map</* local id from ->getID() */ int64_t,
        /* global ID based on full name */ common::model::diagram_element::id_t>
        local_ast_id_map_;

    std::map<int64_t /* local anonymous struct id */,
        std::tuple<std::string /* field name */, common::model::relationship_t,
            common::model::access_t>>
        anonymous_struct_relationships_;
};
}
