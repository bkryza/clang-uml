/**
 * src/sequence_diagram/visitor/call_expression_context.h
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
//
//#include "common/visitor/translation_unit_visitor.h"
//#include "config/config.h"
//#include "sequence_diagram/model/diagram.h"

#include "util/util.h"

#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <stack>

namespace clanguml::sequence_diagram::visitor {

struct call_expression_context {
    call_expression_context();

    void reset();

    void dump();

    bool valid() const;

    clang::ASTContext *get_ast_context();

    void update(clang::CXXRecordDecl *cls);

    void update(clang::ClassTemplateSpecializationDecl *clst);

    void update(clang::ClassTemplateDecl *clst);

    void update(clang::CXXMethodDecl *method);

    void update(clang::FunctionDecl *function);

    void update(clang::FunctionTemplateDecl *function_template);

    bool in_class_method() const;

    bool in_function() const;

    bool in_function_template() const;

    std::int64_t caller_id() const;

    std::int64_t lambda_caller_id() const;

    void set_caller_id(std::int64_t id);

    void enter_lambda_expression(std::int64_t id);

    void leave_lambda_expression();

    clang::IfStmt *current_ifstmt() const
    {
        if (if_stmt_stack_.empty())
            return nullptr;

        return if_stmt_stack_.top();
    }

    void enter_ifstmt(clang::IfStmt *stmt) { return if_stmt_stack_.push(stmt); }

    void leave_ifstmt()
    {
        if (!if_stmt_stack_.empty()) {
            if_stmt_stack_.pop();
            std::stack<clang::IfStmt *>{}.swap(elseif_stmt_stack_);
        }
    }

    void enter_elseifstmt(clang::IfStmt *stmt)
    {
        return elseif_stmt_stack_.push(stmt);
    }

    void leave_elseifstmt()
    {
        if (elseif_stmt_stack_.empty())
            return elseif_stmt_stack_.pop();
    }

    clang::IfStmt *current_elseifstmt() const
    {
        if (elseif_stmt_stack_.empty())
            return nullptr;

        return elseif_stmt_stack_.top();
    }

    clang::CXXRecordDecl *current_class_decl_;
    clang::ClassTemplateDecl *current_class_template_decl_;
    clang::ClassTemplateSpecializationDecl
        *current_class_template_specialization_decl_;
    clang::CXXMethodDecl *current_method_decl_;
    clang::FunctionDecl *current_function_decl_;
    clang::FunctionTemplateDecl *current_function_template_decl_;

    clang::CallExpr *current_function_call_expr_{nullptr};

private:
    std::int64_t current_caller_id_;
    std::stack<std::int64_t> current_lambda_caller_id_;
    std::stack<clang::IfStmt *> if_stmt_stack_;
    std::stack<clang::IfStmt *> elseif_stmt_stack_;
};

}