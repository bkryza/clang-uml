/**
 * @file src/sequence_diagram/visitor/call_expression_context.h
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

#include "common/clang_utils.h"
#include "util/util.h"

#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <stack>

namespace clanguml::sequence_diagram::visitor {

using clanguml::common::eid_t;

/**
 * @brief This class is used to track current context of the call expressions.
 *
 * When traversing AST for call expressions, we need to keep the state of
 * the current context, for instance whether we are in a `for` loop or
 * an `if` block, as well as the current parent of the call expression
 * e.g. a class method or function.
 */
struct call_expression_context {
    /**
     * In Clang, call to a class constructor is represented by
     * `clang::CXXConstructExpr`, which does inherit from `clang::CallExpr`.
     * So to enable to track calls to constructors, we need to be able
     * to add to the call stack either type.
     */
    using callexpr_stack_t = std::variant<std::monostate, clang::CallExpr *,
        clang::CXXConstructExpr *>;

    call_expression_context();

    /**
     * @brief Reset call expression context to the original state.
     */
    void reset();

    /**
     * @brief Verify that the context is in a valid state.
     *
     * Context can only be in a single state (for instance inside a function).
     *
     * @return True, if the context is in a valid state.
     */
    bool valid() const;

    /**
     * @brief
     *
     * @return Current AST context
     */
    clang::ASTContext *get_ast_context() const;

    /**
     * @brief Set the current context to a class.
     *
     * @param cls Class declaration.
     */
    void update(clang::CXXRecordDecl *cls);

    /**
     * @brief Set the current context to a class template specialization.
     *
     * @param clst Class template specialization declaration.
     */
    void update(clang::ClassTemplateSpecializationDecl *clst);

    /**
     * @brief Set the current context to a class template.
     *
     * @param clst Class template declaration.
     */
    void update(clang::ClassTemplateDecl *clst);

    /**
     * @brief Set the current context to a class method.
     *
     * @param method Class method declaration.
     */
    void update(clang::CXXMethodDecl *method);

    /**
     * @brief Set the current context to a function.
     *
     * @param function Function declaration.
     */
    void update(clang::FunctionDecl *function);

    /**
     * @brief Set the current context to a function template.
     *
     * @param function_template Function template declaration.
     */
    void update(clang::FunctionTemplateDecl *function_template);

    /**
     * @brief Set current caller to id of the current participant.
     *
     * @param id Set current caller id.
     */
    void set_caller_id(eid_t id);

    /**
     * @brief Get current caller id
     *
     * @return Id of the current caller participant
     */
    eid_t caller_id() const;

    /**
     * @brief Get the id of the current lambda caller.
     *
     * Since lambdas can be nested within methods and functions, they have
     * a separate caller id field.
     *
     * @return Current lambda caller id, or 0 if current caller is not lambda.
     */
    std::optional<eid_t> lambda_caller_id() const;

    /**
     * @brief Enter a lambda expression
     *
     * @param id Lambda id
     */
    void enter_lambda_expression(eid_t id);

    /**
     * @brief Leave current lambda expression
     */
    void leave_lambda_expression();

    /**
     * @brief Get current `if` statement block
     *
     * @return `if` statement block.
     */
    clang::IfStmt *current_ifstmt() const;

    /**
     * @brief Enter `if` statement block
     *
     * @param stmt `if` statement block
     */
    void enter_ifstmt(clang::IfStmt *stmt);

    /**
     * @brief Leave `if` statement block
     */
    void leave_ifstmt();

    /**
     * @brief Enter `else if` statement block
     *
     * @param stmt `if` statement block
     */
    void enter_elseifstmt(clang::IfStmt *stmt);

    /**
     * @brief Get current `else if` statement block
     *
     * @return `if` statement block.
     */
    clang::IfStmt *current_elseifstmt() const;

    /**
     * @brief Get current loop statement block
     *
     * @return Loop statement block.
     */
    clang::Stmt *current_loopstmt() const;

    /**
     * @brief Enter loop statement block
     *
     * @param stmt Loop statement block
     */
    void enter_loopstmt(clang::Stmt *stmt);

    /**
     * @brief Leave loop statement block
     */
    void leave_loopstmt();

    /**
     * @brief Get current `try` statement block
     *
     * @return `try` statement block.
     */
    clang::Stmt *current_trystmt() const;

    /**
     * @brief Enter `try` statement block
     *
     * @param stmt `try` statement block
     */
    void enter_trystmt(clang::Stmt *stmt);

    /**
     * @brief Leave `try` statement block
     */
    void leave_trystmt();

    /**
     * @brief Get current `switch` statement block
     *
     * @return `switch` statement block.
     */
    clang::SwitchStmt *current_switchstmt() const;

    /**
     * @brief Enter `switch` statement block
     *
     * @param stmt `switch` statement block
     */
    void enter_switchstmt(clang::SwitchStmt *stmt);

    /**
     * @brief Leave `switch` statement block
     */
    void leave_switchstmt();

    /**
     * @brief Get current `:?` statement block
     *
     * @return `:?` statement block.
     */
    clang::ConditionalOperator *current_conditionaloperator() const;

    /**
     * @brief Enter `:?` statement block
     *
     * @param stmt `:?` statement block
     */
    void enter_conditionaloperator(clang::ConditionalOperator *stmt);

    /**
     * @brief Leave `:?` statement block
     */
    void leave_conditionaloperator();

    /**
     * @brief Get current call expression
     *
     * @return Call expression
     */
    callexpr_stack_t current_callexpr() const;

    /**
     * @brief Enter a call expression
     *
     * @param expr Call expression
     */
    void enter_callexpr(clang::CallExpr *expr);

    /**
     * @brief Enter a constructor call expression
     *
     * @param expr Constructor call expression
     */
    void enter_callexpr(clang::CXXConstructExpr *expr);

    /**
     * @brief Leave call expression
     */
    void leave_callexpr();

    /**
     * @brief Check, if a statement is contained in a control statement
     *
     * This method is used to check if `stmt` is contained in control
     * statement of a block, for instance:
     *
     * ```cpp
     *   if(a.method1()) {}
     * ```
     * it will return `true` for `stmt` representing `method1()` call
     * expression.
     *
     * @param stmt Statement
     * @return True, if `stmt` is contained in control expression of a
     *         statement block
     */
    bool is_expr_in_current_control_statement_condition(
        const clang::Stmt *stmt) const;

    /**
     * @brief Print the current call expression stack for debugging.
     */
    void dump();

    clang::CXXRecordDecl *current_class_decl_{nullptr};
    clang::ClassTemplateDecl *current_class_template_decl_{nullptr};
    clang::ClassTemplateSpecializationDecl
        *current_class_template_specialization_decl_{nullptr};
    clang::CXXMethodDecl *current_method_decl_{nullptr};
    clang::FunctionDecl *current_function_decl_{nullptr};
    clang::FunctionTemplateDecl *current_function_template_decl_{nullptr};

private:
    eid_t current_caller_id_{};
    std::stack<eid_t> current_lambda_caller_id_;

    std::stack<callexpr_stack_t> call_expr_stack_;

    std::stack<clang::IfStmt *> if_stmt_stack_;
    std::map<clang::IfStmt *, std::stack<clang::IfStmt *>> elseif_stmt_stacks_;

    std::stack<clang::Stmt *> loop_stmt_stack_;
    std::stack<clang::Stmt *> try_stmt_stack_;
    std::stack<clang::SwitchStmt *> switch_stmt_stack_;
    std::stack<clang::ConditionalOperator *> conditional_operator_stack_;
};

} // namespace clanguml::sequence_diagram::visitor