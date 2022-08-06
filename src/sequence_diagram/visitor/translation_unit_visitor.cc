/**
 * src/sequence_diagram/visitor/translation_unit_visitor.cc
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

#include "translation_unit_visitor.h"

#include "common/model/namespace.h"
#include "cx/util.h"

namespace clanguml::sequence_diagram::visitor {

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::sequence_diagram::model::diagram &diagram,
    const clanguml::config::sequence_diagram &config)
    : source_manager_{sm}
    , diagram_{diagram}
    , config_{config}
    , current_class_decl_{nullptr}
    , current_method_decl_{nullptr}
    , current_function_decl_{nullptr}
{
}

clanguml::sequence_diagram::model::diagram &translation_unit_visitor::diagram()
{
    return diagram_;
}

const clanguml::config::sequence_diagram &
translation_unit_visitor::config() const
{
    return config_;
}

bool translation_unit_visitor::VisitCXXRecordDecl(clang::CXXRecordDecl *cls)
{
    current_class_decl_ = cls;

    return true;
}

bool translation_unit_visitor::VisitCXXMethodDecl(clang::CXXMethodDecl *method)
{
    current_method_decl_ = method;

    return true;
}

bool translation_unit_visitor::VisitFunctionDecl(
    clang::FunctionDecl *function_declaration)
{
    if (!function_declaration->isCXXClassMember())
        current_class_decl_ = nullptr;

    current_function_decl_ = function_declaration;

    return true;
}

bool translation_unit_visitor::VisitCallExpr(clang::CallExpr *expr)
{
    using clanguml::common::model::message_t;
    using clanguml::common::model::namespace_;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    if (expr->isCallToStdMove())
        return true;

    if (expr->isImplicitCXXThis())
        return true;

    if (/*clang::dyn_cast_or_null<clang::CXXOperatorCallExpr>(expr) ||*/
        clang::dyn_cast_or_null<clang::ImplicitCastExpr>(expr))
        return true;

    if (current_class_decl_ &&
        !diagram().should_include(
            current_class_decl_->getQualifiedNameAsString()))
        return true;

    if (current_function_decl_ &&
        !diagram().should_include(
            current_function_decl_->getQualifiedNameAsString()))
        return true;

    message m;
    m.type = message_t::kCall;

    if (current_class_decl_ != nullptr) {
        assert(current_method_decl_ != nullptr);
        m.from = current_class_decl_->getQualifiedNameAsString();
        m.from_usr = current_method_decl_->getID();
    }
    else {
        m.from = current_function_decl_->getQualifiedNameAsString() + "()";
        m.from_usr = current_function_decl_->getID();
    }

    const auto &current_ast_context = current_class_decl_
        ? current_class_decl_->getASTContext()
        : current_function_decl_->getASTContext();

    if (const auto *operator_call_expr =
            clang::dyn_cast_or_null<clang::CXXOperatorCallExpr>(expr);
        operator_call_expr != nullptr) {
        [[maybe_unused]] const auto *callee_method_decl =
            operator_call_expr->getCalleeDecl();
    }
    else if (const auto *method_call_expr =
                 clang::dyn_cast_or_null<clang::CXXMemberCallExpr>(expr);
             method_call_expr != nullptr) {
        const auto *callee_decl = method_call_expr->getMethodDecl()
            ? method_call_expr->getMethodDecl()->getParent()
            : nullptr;

        if (!callee_decl ||
            !diagram().should_include(
                /*namespace_{*/ callee_decl->getQualifiedNameAsString()))
            return true;

        m.to = method_call_expr->getMethodDecl()
                   ->getParent()
                   ->getQualifiedNameAsString();
        m.to_usr = method_call_expr->getMethodDecl()->getID();

        m.message = method_call_expr->getMethodDecl()->getNameAsString();

        m.return_type = method_call_expr->getCallReturnType(current_ast_context)
                            .getAsString();
    }
    else if (const auto *function_call_expr =
                 clang::dyn_cast_or_null<clang::CallExpr>(expr);
             function_call_expr != nullptr) {
        assert(function_call_expr->getCalleeDecl()->getAsFunction());

        m.to = function_call_expr->getCalleeDecl()
                   ->getAsFunction()
                   ->getQualifiedNameAsString() +
            "()";
        m.message = function_call_expr->getCalleeDecl()
                        ->getAsFunction()
                        ->getNameAsString();
        m.to_usr =
            function_call_expr->getCalleeDecl()->getAsFunction()->getID();
        m.return_type =
            function_call_expr->getCallReturnType(current_ast_context)
                .getAsString();
    }
    else {
        return true;
    }

    if (diagram().sequences.find(m.from_usr) == diagram().sequences.end()) {
        activity a;
        a.usr = m.from_usr;
        a.from = m.from;
        diagram().sequences.insert({m.from_usr, std::move(a)});
    }

    LOG_DBG("Found call {} from {} [{}] to {} [{}] ", m.message, m.from,
        m.from_usr, m.to, m.to_usr);

    diagram().sequences[m.from_usr].messages.emplace_back(std::move(m));

    assert(!diagram().sequences.empty());

    return true;
}
}
