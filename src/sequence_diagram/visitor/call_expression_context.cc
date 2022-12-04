/**
 * src/sequence_diagram/visitor/call_expression_context.cc
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

#include "call_expression_context.h"

namespace clanguml::sequence_diagram::visitor {

call_expression_context::call_expression_context()
    : current_class_decl_{nullptr}
    , current_class_template_decl_{nullptr}
    , current_class_template_specialization_decl_{nullptr}
    , current_method_decl_{nullptr}
    , current_function_decl_{nullptr}
    , current_function_template_decl_{nullptr}
    , current_caller_id_{0}
{
}

void call_expression_context::reset()
{
    current_caller_id_ = 0;
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

clang::ASTContext *call_expression_context::get_ast_context()
{
    if (current_class_template_specialization_decl_)
        return &current_class_template_specialization_decl_->getASTContext();

    if (current_class_template_decl_)
        return &current_class_template_decl_->getASTContext();

    if (current_class_decl_)
        return &current_class_decl_->getASTContext();

    if (current_function_template_decl_)
        return &current_function_template_decl_->getASTContext();

    return &current_function_decl_->getASTContext();
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
    if (current_function_template_decl_ &&
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

bool call_expression_context::in_class_method() const
{
    return current_class_decl_ != nullptr;
}

bool call_expression_context::in_function() const
{
    return current_class_decl_ == nullptr && current_function_decl_ != nullptr;
}

bool call_expression_context::in_function_template() const
{
    return current_function_decl_ != nullptr &&
        current_function_template_decl_ != nullptr;
}

std::int64_t call_expression_context::caller_id() const
{
    return current_caller_id_;
}

std::int64_t call_expression_context::lambda_caller_id() const
{
    if (current_lambda_caller_id_.empty())
        return 0;

    return current_lambda_caller_id_.top();
}

void call_expression_context::set_caller_id(std::int64_t id)
{
    LOG_DBG("Setting current caller id to {}", id);
    current_caller_id_ = id;
}

void call_expression_context::enter_lambda_expression(std::int64_t id)
{
    LOG_DBG("Setting current lambda caller id to {}", id);

    assert(id != 0);

    current_lambda_caller_id_.push(id);
}

void call_expression_context::leave_lambda_expression()
{
    assert(!current_lambda_caller_id_.empty());

    LOG_DBG("Leaving current lambda expression id to {}",
        current_lambda_caller_id_.top());

    current_lambda_caller_id_.pop();
}

}