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

#include "common/visitor/translation_unit_visitor.h"
#include "config/config.h"
#include "sequence_diagram/model/diagram.h"

#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

namespace clanguml::sequence_diagram::visitor {

std::string to_string(const clang::FunctionTemplateDecl *decl);

struct call_expression_context {
    call_expression_context()
        : current_class_decl_{nullptr}
        , current_class_template_decl_{nullptr}
        , current_method_decl_{nullptr}
        , current_function_decl_{nullptr}
        , current_function_template_decl_{nullptr}
    {
    }

    void reset()
    {
        current_caller_id_ = 0;
        current_class_decl_ = nullptr;
        current_class_template_decl_ = nullptr;
        current_method_decl_ = nullptr;
        current_function_decl_ = nullptr;
        current_function_template_decl_ = nullptr;
    }

    void dump()
    {
        LOG_DBG("current_caller_id_ = {}", current_caller_id_);
        LOG_DBG("current_class_decl_ = {}", (void *)current_class_decl_);
        LOG_DBG("current_class_template_decl_ = {}",
            (void *)current_class_template_decl_);
        LOG_DBG("current_method_decl_ = {}", (void *)current_method_decl_);
        LOG_DBG("current_function_decl_ = {}", (void *)current_function_decl_);
        LOG_DBG("current_function_template_decl_ = {}",
            (void *)current_function_template_decl_);
    }

    bool valid() const
    {
        return (current_class_decl_ != nullptr) ||
            (current_class_template_decl_ != nullptr) ||
            (current_method_decl_ != nullptr) ||
            (current_function_decl_ != nullptr) ||
            (current_function_template_decl_ != nullptr);
    }

    clang::ASTContext *get_ast_context()
    {
        if (current_class_template_decl_)
            return &current_class_template_decl_->getASTContext();

        if (current_class_decl_)
            return &current_class_decl_->getASTContext();

        if (current_function_template_decl_)
            return &current_function_template_decl_->getASTContext();

        return &current_function_decl_->getASTContext();
    }

    void update(clang::CXXRecordDecl *cls) { current_class_decl_ = cls; }

    void update(clang::ClassTemplateDecl *clst)
    {
        current_class_template_decl_ = clst;
    }

    void update(clang::CXXMethodDecl *method) { current_method_decl_ = method; }

    void update(clang::FunctionDecl *function)
    {
        if (!function->isCXXClassMember())
            reset();

        current_function_decl_ = function;

        // Check if this function is a part of template function declaration,
        // If no - reset the current_function_template_decl_
        if (current_function_template_decl_ &&
            current_function_template_decl_->getQualifiedNameAsString() !=
                function->getQualifiedNameAsString()) {
            current_function_template_decl_ = nullptr;
        }
        //        else {
        //            call_expression_context_.current_class_method_ =
        //                process_class_method(method);
        //        }
    }

    void update(clang::FunctionTemplateDecl *function_template)
    {
        current_function_template_decl_ = function_template;

        if (!function_template->isCXXClassMember())
            current_class_decl_ = nullptr;

        current_function_template_decl_ = function_template;
    }

    bool in_class_method() const { return current_class_decl_ != nullptr; }

    bool in_function() const
    {
        return current_class_decl_ == nullptr &&
            current_function_decl_ != nullptr;
    }

    bool in_function_template() const
    {
        return current_function_decl_ != nullptr &&
            current_function_template_decl_ != nullptr;
    }

    std::int64_t caller_id() const { return current_caller_id_; }

    void set_caller_id(std::int64_t id)
    {
        LOG_DBG("Setting current caller id to {}", id);
        current_caller_id_ = id;
    }

    clang::CXXRecordDecl *current_class_decl_;
    clang::ClassTemplateDecl *current_class_template_decl_;
    clang::CXXMethodDecl *current_method_decl_;
    clang::FunctionDecl *current_function_decl_;
    clang::FunctionTemplateDecl *current_function_template_decl_;

private:
    std::int64_t current_caller_id_;
};

class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor>,
      public common::visitor::translation_unit_visitor {
public:
    translation_unit_visitor(clang::SourceManager &sm,
        clanguml::sequence_diagram::model::diagram &diagram,
        const clanguml::config::sequence_diagram &config);

    virtual bool VisitCallExpr(clang::CallExpr *expr);

    virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl *method);

    virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *cls);

    virtual bool VisitClassTemplateDecl(clang::ClassTemplateDecl *cls);

    virtual bool VisitClassTemplateSpecializationDecl(
        clang::ClassTemplateSpecializationDecl *cls);

    virtual bool VisitFunctionDecl(clang::FunctionDecl *function_declaration);

    bool VisitFunctionTemplateDecl(
        clang::FunctionTemplateDecl *function_declaration);

    clanguml::sequence_diagram::model::diagram &diagram();

    const clanguml::config::sequence_diagram &config() const;

    void finalize() { }

    /// Store the mapping from local clang entity id (obtained using
    /// getID()) method to clang-uml global id
    void set_ast_local_id(
        int64_t local_id, common::model::diagram_element::id_t global_id);

    /// Retrieve the global clang-uml entity id based on the clang local id
    std::optional<common::model::diagram_element::id_t> get_ast_local_id(
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

    // Reference to the output diagram model
    clanguml::sequence_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::sequence_diagram &config_;

    call_expression_context call_expression_context_;

    std::map<common::model::diagram_element::id_t,
        std::unique_ptr<clanguml::sequence_diagram::model::class_>>
        forward_declarations_;

    std::map<int64_t, common::model::diagram_element::id_t> local_ast_id_map_;

    std::map<int64_t /* local anonymous struct id */,
        std::tuple<std::string /* field name */, common::model::relationship_t,
            common::model::access_t>>
        anonymous_struct_relationships_;
};

}
