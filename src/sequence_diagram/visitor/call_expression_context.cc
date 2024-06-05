/**
 * @file src/sequence_diagram/visitor/call_expression_context.cc
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

#include "call_expression_context.h"

namespace clanguml::sequence_diagram::visitor {

call_expression_context::call_expression_context() = default;

void call_expression_context::reset()
{
    current_caller_id_ = eid_t{};
    current_class_decl_ = nullptr;
    current_class_template_decl_ = nullptr;
    current_class_template_specialization_decl_ = nullptr;
    current_method_decl_ = nullptr;
    current_function_decl_ = nullptr;
    current_function_template_decl_ = nullptr;
}

void call_expression_context::dump()
{
    LOG_DBG("current_caller_id_ = {}", current_caller_id_);
    LOG_DBG("current_class_decl_ = {}", (void *)current_class_decl_);
    LOG_DBG("current_class_template_decl_ = {}",
        (void *)current_class_template_decl_);
    LOG_DBG("current_class_template_specialization_decl_ = {}",
        (void *)current_class_template_specialization_decl_);
    LOG_DBG("current_method_decl_ = {}", (void *)current_method_decl_);
    LOG_DBG("current_function_decl_ = {}", (void *)current_function_decl_);
    LOG_DBG("current_function_template_decl_ = {}",
        (void *)current_function_template_decl_);
}

bool call_expression_context::valid() const
{
    return (current_class_decl_ != nullptr) ||
        (current_class_template_decl_ != nullptr) ||
        (current_class_template_specialization_decl_ != nullptr) ||
        (current_method_decl_ != nullptr) ||
        (current_function_decl_ != nullptr) ||
        (current_function_template_decl_ != nullptr);
}

clang::ASTContext *call_expression_context::get_ast_context() const
{
    if (current_class_template_specialization_decl_ != nullptr)
        return &current_class_template_specialization_decl_->getASTContext();

    if (current_class_template_decl_ != nullptr)
        return &current_class_template_decl_->getASTContext();

    if (current_class_decl_ != nullptr)
        return &current_class_decl_->getASTContext();

    if (current_function_template_decl_ != nullptr)
        return &current_function_template_decl_->getASTContext();

    if (current_function_decl_ != nullptr) {
        return &current_function_decl_->getASTContext();
    }

    if (current_method_decl_ != nullptr) {
        return &current_method_decl_->getASTContext();
    }

    return nullptr;
}

void call_expression_context::update(clang::CXXRecordDecl *cls)
{
    current_class_decl_ = cls;
}

void call_expression_context::update(
    clang::ClassTemplateSpecializationDecl *clst)
{
    current_class_template_specialization_decl_ = clst;
}

void call_expression_context::update(clang::ClassTemplateDecl *clst)
{
    current_class_template_decl_ = clst;
}

void call_expression_context::update(clang::CXXMethodDecl *method)
{
    current_method_decl_ = method;
}

void call_expression_context::update(clang::FunctionDecl *function)
{
    if (!function->isCXXClassMember())
        reset();

    current_function_decl_ = function;

    // Check if this function is a part of template function declaration,
    // If no - reset the current_function_template_decl_
    if ((current_function_template_decl_ != nullptr) &&
        current_function_template_decl_->getQualifiedNameAsString() !=
            function->getQualifiedNameAsString()) {
        current_function_template_decl_ = nullptr;
    }
}

void call_expression_context::update(
    clang::FunctionTemplateDecl *function_template)
{
    current_function_template_decl_ = function_template;

    if (!function_template->isCXXClassMember())
        current_class_decl_ = nullptr;

    current_function_template_decl_ = function_template;
}

eid_t call_expression_context::caller_id() const
{
    if (lambda_caller_id().has_value())
        return *lambda_caller_id(); // NOLINT

    return current_caller_id_;
}

std::optional<eid_t> call_expression_context::lambda_caller_id() const
{
    if (current_lambda_caller_id_.empty())
        return {};

    return current_lambda_caller_id_.top();
}

void call_expression_context::set_caller_id(eid_t id)
{
    LOG_DBG("Setting current caller id to {}", id);
    current_caller_id_ = id;
}

void call_expression_context::enter_lambda_expression(eid_t id)
{
    LOG_DBG("Setting current lambda caller id to {}", id);

    assert(id.value() != 0);

    current_lambda_caller_id_.emplace(id);
}

void call_expression_context::leave_lambda_expression()
{
    if (current_lambda_caller_id_.empty())
        return;

    LOG_DBG("Leaving current lambda expression id to {}",
        current_lambda_caller_id_.top());

    current_lambda_caller_id_.pop();
}

clang::IfStmt *call_expression_context::current_ifstmt() const
{
    if (if_stmt_stack_.empty())
        return nullptr;

    return if_stmt_stack_.top();
}

void call_expression_context::enter_ifstmt(clang::IfStmt *stmt)
{
    if_stmt_stack_.emplace(stmt);
}

void call_expression_context::leave_ifstmt()
{
    if (!if_stmt_stack_.empty()) {
        elseif_stmt_stacks_.erase(current_ifstmt());
        if_stmt_stack_.pop();
    }
}

void call_expression_context::enter_elseifstmt(clang::IfStmt *stmt)
{
    assert(current_ifstmt() != nullptr);

    elseif_stmt_stacks_[current_ifstmt()].emplace(stmt);
}

clang::IfStmt *call_expression_context::current_elseifstmt() const
{
    assert(current_ifstmt() != nullptr);

    if (elseif_stmt_stacks_.count(current_ifstmt()) == 0 ||
        elseif_stmt_stacks_.at(current_ifstmt()).empty())
        return nullptr;

    return elseif_stmt_stacks_.at(current_ifstmt()).top();
}

clang::Stmt *call_expression_context::current_loopstmt() const
{
    if (loop_stmt_stack_.empty())
        return nullptr;

    return loop_stmt_stack_.top();
}

void call_expression_context::enter_loopstmt(clang::Stmt *stmt)
{
    loop_stmt_stack_.emplace(stmt);
}

void call_expression_context::leave_loopstmt()
{
    if (!loop_stmt_stack_.empty())
        return loop_stmt_stack_.pop();
}

call_expression_context::callexpr_stack_t
call_expression_context::current_callexpr() const
{
    if (call_expr_stack_.empty())
        return {};

    return call_expr_stack_.top();
}

void call_expression_context::enter_callexpr(clang::CallExpr *expr)
{
    call_expr_stack_.emplace(expr);
}

void call_expression_context::enter_callexpr(clang::CXXConstructExpr *expr)
{
    call_expr_stack_.emplace(expr);
}

void call_expression_context::leave_callexpr()
{
    if (!call_expr_stack_.empty()) {
        return call_expr_stack_.pop();
    }
}

clang::Stmt *call_expression_context::current_trystmt() const
{
    if (try_stmt_stack_.empty())
        return nullptr;

    return try_stmt_stack_.top();
}

void call_expression_context::enter_trystmt(clang::Stmt *stmt)
{
    try_stmt_stack_.emplace(stmt);
}

void call_expression_context::leave_trystmt()
{
    if (try_stmt_stack_.empty())
        try_stmt_stack_.pop();
}

clang::SwitchStmt *call_expression_context::current_switchstmt() const
{
    if (switch_stmt_stack_.empty())
        return nullptr;

    return switch_stmt_stack_.top();
}

void call_expression_context::enter_switchstmt(clang::SwitchStmt *stmt)
{
    switch_stmt_stack_.emplace(stmt);
}

void call_expression_context::leave_switchstmt()
{
    if (switch_stmt_stack_.empty())
        switch_stmt_stack_.pop();
}

clang::ConditionalOperator *
call_expression_context::current_conditionaloperator() const
{
    if (conditional_operator_stack_.empty())
        return nullptr;

    return conditional_operator_stack_.top();
}

void call_expression_context::enter_conditionaloperator(
    clang::ConditionalOperator *stmt)
{
    conditional_operator_stack_.emplace(stmt);
}

void call_expression_context::leave_conditionaloperator()
{
    if (!conditional_operator_stack_.empty())
        conditional_operator_stack_.pop();
}

bool call_expression_context::is_expr_in_current_control_statement_condition(
    const clang::Stmt *stmt) const
{
    if (current_ifstmt() != nullptr) {
        if (common::is_subexpr_of(current_ifstmt()->getCond(), stmt))
            return true;

        if (const auto *condition_decl_stmt = current_ifstmt()->getInit();
            condition_decl_stmt != nullptr) {
            if (common::is_subexpr_of(condition_decl_stmt, stmt))
                return true;
        }

        if (current_elseifstmt() != nullptr) {
            if (common::is_subexpr_of(current_elseifstmt()->getCond(), stmt))
                return true;
        }
    }

    if (current_conditionaloperator() != nullptr) {
        if (common::is_subexpr_of(
                current_conditionaloperator()->getCond(), stmt))
            return true;
    }

    if (const auto *loop_stmt = current_loopstmt(); loop_stmt != nullptr) {
        if (const auto *for_stmt = clang::dyn_cast<clang::ForStmt>(loop_stmt);
            for_stmt != nullptr) {
            if (common::is_subexpr_of(for_stmt->getCond(), stmt)) {
                return true;
            }
            if (common::is_subexpr_of(for_stmt->getInit(), stmt)) {
                return true;
            }
            if (common::is_subexpr_of(for_stmt->getInc(), stmt)) {
                return true;
            }
        }

        if (const auto *range_for_stmt =
                clang::dyn_cast<clang::CXXForRangeStmt>(loop_stmt);
            range_for_stmt != nullptr) {
            if (common::is_subexpr_of(range_for_stmt->getRangeInit(), stmt)) {
                return true;
            }
        }

        if (const auto *while_stmt =
                clang::dyn_cast<clang::WhileStmt>(loop_stmt);
            while_stmt != nullptr) {
            if (common::is_subexpr_of(while_stmt->getCond(), stmt)) {
                return true;
            }
        }

        if (const auto *do_stmt = clang::dyn_cast<clang::DoStmt>(loop_stmt);
            do_stmt != nullptr) {
            if (common::is_subexpr_of(do_stmt->getCond(), stmt)) {
                return true;
            }
        }

        if (current_conditionaloperator() != nullptr) {
            if (common::is_subexpr_of(
                    current_conditionaloperator()->getCond(), stmt)) {
                return true;
            }
        }
    }

    return false;
}

} // namespace clanguml::sequence_diagram::visitor