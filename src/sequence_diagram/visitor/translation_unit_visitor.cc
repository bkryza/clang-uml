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

std::string to_string(const clang::FunctionTemplateDecl *decl)
{
    std::vector<std::string> template_parameters;
    // Handle template function
    for (const auto *parameter : *decl->getTemplateParameters()) {
        if (clang::dyn_cast_or_null<clang::TemplateTypeParmDecl>(parameter)) {
            const auto *template_type_parameter =
                clang::dyn_cast_or_null<clang::TemplateTypeParmDecl>(parameter);

            std::string template_parameter{
                template_type_parameter->getNameAsString()};

            if (template_type_parameter->isParameterPack())
                template_parameter += "...";

            template_parameters.emplace_back(std::move(template_parameter));
        }
        else {
            // TODO
        }
    }
    return fmt::format("{}<{}>({})", decl->getQualifiedNameAsString(),
        fmt::join(template_parameters, ","), "");
}

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::sequence_diagram::model::diagram &diagram,
    const clanguml::config::sequence_diagram &config)
    : source_manager_{sm}
    , diagram_{diagram}
    , config_{config}
    , current_class_decl_{nullptr}
    , current_method_decl_{nullptr}
    , current_function_decl_{nullptr}
    , current_function_template_decl_{nullptr}

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

    // Check if this function is a part of template function declaration,
    // If no - reset the current_function_template_decl_
    if (current_function_template_decl_ &&
        current_function_template_decl_->getQualifiedNameAsString() !=
            function_declaration->getQualifiedNameAsString())
        current_function_template_decl_ = nullptr;

    LOG_DBG("Visiting function {}",
        current_function_decl_->getQualifiedNameAsString());

    return true;
}

bool translation_unit_visitor::VisitFunctionTemplateDecl(
    clang::FunctionTemplateDecl *function_declaration)
{
    if (!function_declaration->isCXXClassMember())
        current_class_decl_ = nullptr;

    current_function_template_decl_ = function_declaration;

    LOG_DBG("Visiting template function {}",
        current_function_template_decl_->getQualifiedNameAsString());

    return true;
}

bool translation_unit_visitor::VisitCallExpr(clang::CallExpr *expr)
{
    using clanguml::common::model::message_t;
    using clanguml::common::model::namespace_;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    // Skip casts, moves and such
    if (expr->isCallToStdMove())
        return true;

    if (expr->isImplicitCXXThis())
        return true;

    if (clang::dyn_cast_or_null<clang::ImplicitCastExpr>(expr))
        return true;

    // Skip if current class was excluded in the config

    if (current_class_decl_) {
        std::string current_class_qualified_name =
            current_class_decl_->getQualifiedNameAsString();
        if (!diagram().should_include(current_class_qualified_name))
            return true;
    }

    // Skip if current function was excluded in the config
    if (current_function_decl_) {
        std::string current_function_qualified_name =
            current_function_decl_->getQualifiedNameAsString();
        if (!diagram().should_include(current_function_qualified_name))
            return true;
    }

    message m;
    m.type = message_t::kCall;

    if (current_class_decl_ != nullptr) {
        // Handle call expression within some class method
        assert(current_method_decl_ != nullptr);
        m.from = current_class_decl_->getQualifiedNameAsString();
        m.from_usr = current_method_decl_->getID();
    }
    else if (current_function_template_decl_ != nullptr &&
        current_function_decl_ != nullptr) {

        m.from = to_string(current_function_template_decl_);
        m.from_usr = current_function_template_decl_->getID();
    }
    else if (current_function_decl_ != nullptr) {
        // Handle call expression within free function
        m.from = current_function_decl_->getQualifiedNameAsString() + "()";
        m.from_usr = current_function_decl_->getID();
    }
    else {
        m.from = current_function_template_decl_->getQualifiedNameAsString();
        std::vector<std::string> params;
        for (const auto &template_parameter :
            *current_function_template_decl_->getTemplateParameters()) {
            params.push_back(template_parameter->getNameAsString());
        }
        m.from += fmt::format("<{}>", fmt::join(params, ","));
    }

    const auto &current_ast_context = current_class_decl_
        ? current_class_decl_->getASTContext()
        : current_function_decl_->getASTContext();

    if (const auto *operator_call_expr =
            clang::dyn_cast_or_null<clang::CXXOperatorCallExpr>(expr);
        operator_call_expr != nullptr) {
        // TODO: Handle C++ operator calls
    }
    else if (const auto *method_call_expr =
                 clang::dyn_cast_or_null<clang::CXXMemberCallExpr>(expr);
             method_call_expr != nullptr) {

        // Get callee declaration as methods parent
        const auto *method_decl = method_call_expr->getMethodDecl();
        const auto *callee_decl =
            method_decl ? method_decl->getParent() : nullptr;

        if (!(callee_decl &&
                diagram().should_include(
                    callee_decl->getQualifiedNameAsString())))
            return true;

        m.to = callee_decl->getQualifiedNameAsString();
        m.to_usr = method_decl->getID();
        m.message = method_decl->getNameAsString();
        m.return_type = method_call_expr->getCallReturnType(current_ast_context)
                            .getAsString();
    }
    else if (const auto *function_call_expr =
                 clang::dyn_cast_or_null<clang::CallExpr>(expr);
             function_call_expr != nullptr) {
        auto *callee_decl = function_call_expr->getCalleeDecl();

        if (callee_decl == nullptr) {
            LOG_DBG("Cannot get callee declaration - trying direct callee...");
            callee_decl = function_call_expr->getDirectCallee();
        }

        if (!callee_decl) {
            if (clang::dyn_cast_or_null<clang::UnresolvedLookupExpr>(
                    function_call_expr->getCallee())) {
                // This is probably a template
                auto *unresolved_expr =
                    clang::dyn_cast_or_null<clang::UnresolvedLookupExpr>(
                        function_call_expr->getCallee());

                if (unresolved_expr) {
                    for (const auto *decl : unresolved_expr->decls()) {
                        if (clang::dyn_cast_or_null<
                                clang::FunctionTemplateDecl>(decl)) {

                            auto *ftd = clang::dyn_cast_or_null<
                                clang::FunctionTemplateDecl>(decl);

                            m.to = to_string(ftd);
                            m.message = to_string(ftd);
                            m.to_usr = ftd->getID();
                            break;
                        }
                    }
                }
            }
        }
        else {
            if (!callee_decl)
                return true;

            if (callee_decl->isTemplateDecl())
                LOG_DBG("Call to template function!!!!");

            const auto *callee_function = callee_decl->getAsFunction();

            if (!callee_function)
                return true;

            m.to = callee_function->getQualifiedNameAsString() + "()";
            m.message = callee_function->getNameAsString();
            m.to_usr = callee_function->getID();
        }
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
