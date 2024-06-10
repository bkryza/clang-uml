/**
 * @file src/sequence_diagram/visitor/translation_unit_visitor.cc
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

#include "translation_unit_visitor.h"

#include "common/clang_utils.h"
#include "common/model/namespace.h"
#include "sequence_diagram/model/participant.h"

namespace clanguml::sequence_diagram::visitor {

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::sequence_diagram::model::diagram &diagram,
    const clanguml::config::sequence_diagram &config)
    : visitor_specialization_t{sm, diagram, config}
    , template_builder_{diagram, config, *this}
{
}

std::unique_ptr<sequence_diagram::model::class_>
translation_unit_visitor::create_element(
    const clang::NamedDecl * /*decl*/) const
{
    return std::make_unique<sequence_diagram::model::class_>(
        config().using_namespace());
}

bool translation_unit_visitor::shouldVisitTemplateInstantiations()
{
    return true;
}

call_expression_context &translation_unit_visitor::context()
{
    return call_expression_context_;
}

const call_expression_context &translation_unit_visitor::context() const
{
    return call_expression_context_;
}

bool translation_unit_visitor::VisitCXXRecordDecl(
    clang::CXXRecordDecl *declaration)
{
    if (!should_include(declaration))
        return true;

    // Skip this class if it's parent template is already in the model
    if (declaration->isTemplated() &&
        declaration->getDescribedTemplate() != nullptr) {
        if (get_unique_id(eid_t{declaration->getDescribedTemplate()->getID()}))
            return true;
    }

    // TODO: Add support for classes defined in function/method bodies
    if (declaration->isLocalClass() != nullptr)
        return true;

    LOG_TRACE("Visiting class declaration at {}",
        declaration->getBeginLoc().printToString(source_manager()));

    // Build the class declaration and store it in the diagram, even
    // if we don't need it for any of the participants of this diagram
    auto class_model_ptr = create_class_model(declaration);

    if (!class_model_ptr)
        return true;

    context().reset();

    const auto class_id = class_model_ptr->id();

    set_unique_id(declaration->getID(), class_id);

    auto &class_model =
        diagram()
            .get_participant<sequence_diagram::model::class_>(class_id)
            .has_value()
        ? *diagram()
               .get_participant<sequence_diagram::model::class_>(class_id)
               .get()
        : *class_model_ptr;

    if (!declaration->isCompleteDefinition()) {
        forward_declarations_.emplace(class_id, std::move(class_model_ptr));
        return true;
    }

    forward_declarations_.erase(class_id);

    if (diagram().should_include(class_model)) {
        LOG_DBG("Adding class participant {} with id {}",
            class_model.full_name(false), class_model.id());

        assert(class_model.id() == class_id);

        context().set_caller_id(class_id);
        context().update(declaration);

        diagram().add_participant(std::move(class_model_ptr));
    }
    else {
        LOG_DBG(
            "Skipping class {} with id {}", class_model.full_name(), class_id);
    }

    return true;
}

bool translation_unit_visitor::VisitClassTemplateDecl(
    clang::ClassTemplateDecl *declaration)
{
    if (!should_include(declaration))
        return true;

    LOG_TRACE("Visiting class template declaration {} at {} [{}]",
        declaration->getQualifiedNameAsString(),
        declaration->getLocation().printToString(source_manager()),
        (void *)declaration);

    auto class_model_ptr = create_class_model(declaration->getTemplatedDecl());

    if (!class_model_ptr)
        return true;

    tbuilder().build_from_template_declaration(*class_model_ptr, *declaration);

    const auto class_full_name = class_model_ptr->full_name(false);
    const auto id = common::to_id(class_full_name);

    // Override the id with the template id, for now we don't care about the
    // underlying templated class id
    class_model_ptr->set_id(id);

    set_unique_id(declaration->getID(), id);

    if (!declaration->getTemplatedDecl()->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(class_model_ptr));
        return true;
    }
    forward_declarations_.erase(id);

    if (diagram().should_include(*class_model_ptr)) {
        LOG_DBG("Adding class template participant {} with id {}",
            class_full_name, id);

        context().set_caller_id(id);
        context().update(declaration);

        diagram().add_participant(std::move(class_model_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitClassTemplateSpecializationDecl(
    clang::ClassTemplateSpecializationDecl *declaration)
{
    if (!should_include(declaration))
        return true;

    LOG_TRACE("Visiting template specialization declaration {} at {}",
        declaration->getQualifiedNameAsString(),
        declaration->getLocation().printToString(source_manager()));

    // TODO: Add support for classes defined in function/method bodies
    if (declaration->isLocalClass() != nullptr)
        return true;

    auto template_specialization_ptr =
        process_class_template_specialization(declaration);

    if (!template_specialization_ptr)
        return true;

    const auto class_full_name = template_specialization_ptr->full_name(false);
    const auto id = common::to_id(class_full_name);

    template_specialization_ptr->set_id(id);

    set_unique_id(declaration->getID(), id);

    if (!declaration->isCompleteDefinition()) {
        forward_declarations_.emplace(
            id, std::move(template_specialization_ptr));
        return true;
    }
    forward_declarations_.erase(id);

    if (diagram().should_include(*template_specialization_ptr)) {
        LOG_DBG(
            "Adding class template specialization participant {} with id {}",
            class_full_name, id);

        context().set_caller_id(id);
        context().update(declaration);

        diagram().add_participant(std::move(template_specialization_ptr));
    }

    return true;
}

bool translation_unit_visitor::TraverseCXXMethodDecl(
    clang::CXXMethodDecl *declaration)
{
    // We need to backup the context, since other methods or functions can
    // be traversed during this traversal (e.g. template function/method
    // specializations)
    auto context_backup = context();

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXMethodDecl(
        declaration);

    call_expression_context_ = context_backup;

    return true;
}

bool translation_unit_visitor::VisitCXXMethodDecl(
    clang::CXXMethodDecl *declaration)
{
    if (!should_include(declaration))
        return true;

    if (!declaration->isThisDeclarationADefinition()) {
        if (auto *declaration_definition = declaration->getDefinition();
            declaration_definition != nullptr) {
            if (auto *method_definition = clang::dyn_cast<clang::CXXMethodDecl>(
                    declaration_definition);
                method_definition != nullptr) {
                LOG_DBG("Calling VisitCXXMethodDecl recursively for forward "
                        "declaration");

                return VisitCXXMethodDecl(method_definition);
            }
        }
    }

    LOG_TRACE("Visiting method {} in class {} [{}]",
        declaration->getQualifiedNameAsString(),
        declaration->getParent()->getQualifiedNameAsString(),
        (void *)declaration->getParent());

    context().update(declaration);

    auto method_model_ptr = create_method_model(declaration);

    if (!method_model_ptr)
        return true;

    process_comment(*declaration, *method_model_ptr);

    set_source_location(*declaration, *method_model_ptr);

    const auto method_full_name = method_model_ptr->full_name(false);

    method_model_ptr->set_id(common::to_id(method_full_name));

    // Callee methods in call expressions are referred to by first declaration
    // id, so they should both be mapped to method_model
    if (declaration->isThisDeclarationADefinition()) {
        set_unique_id(
            declaration->getFirstDecl()->getID(), method_model_ptr->id());
    }

    set_unique_id(declaration->getID(), method_model_ptr->id());

    LOG_TRACE("Set id {} --> {} for method name {} [{}]", declaration->getID(),
        method_model_ptr->id(), method_full_name,
        declaration->isThisDeclarationADefinition());

    context().update(declaration);

    context().set_caller_id(method_model_ptr->id());

    diagram().add_participant(std::move(method_model_ptr));

    return true;
}

bool translation_unit_visitor::TraverseFunctionDecl(
    clang::FunctionDecl *declaration)
{
    // We need to backup the context, since other methods or functions can
    // be traversed during this traversal (e.g. template function/method
    // specializations)
    auto context_backup = context();

    RecursiveASTVisitor<translation_unit_visitor>::TraverseFunctionDecl(
        declaration);

    call_expression_context_ = context_backup;

    return true;
}

bool translation_unit_visitor::VisitFunctionDecl(
    clang::FunctionDecl *declaration)
{
    if (declaration->isCXXClassMember())
        return true;

    if (!should_include(declaration))
        return true;

    if (!declaration->isThisDeclarationADefinition()) {
        if (auto *declaration_definition = declaration->getDefinition();
            declaration_definition != nullptr)
            return VisitFunctionDecl(
                static_cast<clang::FunctionDecl *>(declaration_definition));
    }

    LOG_TRACE("Visiting function declaration {} at {}",
        declaration->getQualifiedNameAsString(),
        declaration->getLocation().printToString(source_manager()));

    if (declaration->isTemplated()) {
        if (declaration->getDescribedTemplate() != nullptr) {
            // If the described templated of this function is already in the
            // model skip it:
            if (get_unique_id(
                    eid_t{declaration->getDescribedTemplate()->getID()}))
                return true;
        }
    }

    std::unique_ptr<model::function> function_model_ptr{};

    if (declaration->isFunctionTemplateSpecialization()) {
        function_model_ptr =
            build_function_template_instantiation(*declaration);
    }
    else {
        function_model_ptr = build_function_model(*declaration);
    }

    if (!function_model_ptr)
        return true;

    function_model_ptr->set_id(
        common::to_id(function_model_ptr->full_name(false)));

    function_model_ptr->is_void(declaration->getReturnType()->isVoidType());

    function_model_ptr->is_operator(declaration->isOverloadedOperator());

    function_model_ptr->is_cuda_kernel(
        common::has_attr(declaration, clang::attr::CUDAGlobal));

    function_model_ptr->is_cuda_device(
        common::has_attr(declaration, clang::attr::CUDADevice));

    context().update(declaration);

    context().set_caller_id(function_model_ptr->id());

    if (declaration->isThisDeclarationADefinition()) {
        set_unique_id(
            declaration->getFirstDecl()->getID(), function_model_ptr->id());
    }

    set_unique_id(declaration->getID(), function_model_ptr->id());

    process_comment(*declaration, *function_model_ptr);

    set_source_location(*declaration, *function_model_ptr);

    diagram().add_participant(std::move(function_model_ptr));

    return true;
}

bool translation_unit_visitor::VisitFunctionTemplateDecl(
    clang::FunctionTemplateDecl *declaration)
{
    if (!should_include(declaration))
        return true;

    const auto function_name = declaration->getQualifiedNameAsString();

    LOG_TRACE("Visiting function template declaration {} at {}", function_name,
        declaration->getLocation().printToString(source_manager()));

    auto function_template_model = build_function_template(*declaration);

    process_comment(*declaration, *function_template_model);

    set_source_location(*declaration, *function_template_model);
    set_owning_module(*declaration, *function_template_model);

    function_template_model->is_void(
        declaration->getAsFunction()->getReturnType()->isVoidType());

    function_template_model->set_id(
        common::to_id(function_template_model->full_name(false)));

    function_template_model->is_operator(
        declaration->getAsFunction()->isOverloadedOperator());

    context().update(declaration);

    context().set_caller_id(function_template_model->id());

    set_unique_id(declaration->getID(), function_template_model->id());

    diagram().add_participant(std::move(function_template_model));

    return true;
}

bool translation_unit_visitor::VisitLambdaExpr(clang::LambdaExpr *expr)
{
    if (!should_include(expr))
        return true;

    const auto lambda_full_name =
        expr->getLambdaClass()->getCanonicalDecl()->getNameAsString();

    LOG_TRACE("Visiting lambda expression {} at {} [caller_id = {}]",
        lambda_full_name, expr->getBeginLoc().printToString(source_manager()),
        context().caller_id());

    LOG_TRACE("Lambda call operator ID {} - lambda class ID {}, class call "
              "operator ID {}",
        expr->getCallOperator()->getID(), expr->getLambdaClass()->getID(),
        expr->getLambdaClass()->getLambdaCallOperator()->getID());

    // Create lambda class participant
    auto *cls = expr->getLambdaClass();
    auto lambda_class_model_ptr = create_class_model(cls);

    if (!lambda_class_model_ptr)
        return true;

    const auto cls_id = lambda_class_model_ptr->id();

    set_unique_id(cls->getID(), cls_id);

    auto lambda_method_model_ptr =
        create_lambda_method_model(expr->getCallOperator());

    lambda_method_model_ptr->set_class_id(cls_id);

    // If this is a nested lambda, prepend the parent lambda name to this lambda
    auto lambda_class_full_name = lambda_class_model_ptr->full_name(false);
    lambda_method_model_ptr->set_class_full_name(lambda_class_full_name);

    diagram().add_participant(std::move(lambda_class_model_ptr));

    lambda_method_model_ptr->set_id(
        common::to_id(get_participant(cls_id).value().full_name(false) +
            "::" + lambda_method_model_ptr->full_name_no_ns()));

    get_participant<model::class_>(cls_id).value().set_lambda_operator_id(
        lambda_method_model_ptr->id());

    // If lambda expression is in an argument to a method/function, and that
    // method function would be excluded by filters
    if (std::holds_alternative<clang::CallExpr *>(
            context().current_callexpr()) &&
        (!context().lambda_caller_id().has_value())) {
        using clanguml::common::model::message_t;
        using clanguml::sequence_diagram::model::message;

        message m{message_t::kCall, context().caller_id()};
        set_source_location(*expr, m);
        m.set_from(context().caller_id());
        m.set_to(lambda_method_model_ptr->id());

        diagram().add_active_participant(m.from());
        diagram().add_active_participant(m.to());

        LOG_DBG("Found call {} from {} [{}] to {} [{}]", m.message_name(),
            m.from(), m.from(), m.to(), m.to());

        push_message(std::get<clang::CallExpr *>(context().current_callexpr()),
            std::move(m));
    }

    context().enter_lambda_expression(lambda_method_model_ptr->id());

    set_unique_id(
        expr->getCallOperator()->getID(), lambda_method_model_ptr->id());

    diagram().add_participant(std::move(lambda_method_model_ptr));

    [[maybe_unused]] const auto is_generic_lambda = expr->isGenericLambda();

    return true;
}

bool translation_unit_visitor::TraverseLambdaExpr(clang::LambdaExpr *expr)
{
    RecursiveASTVisitor<translation_unit_visitor>::TraverseLambdaExpr(expr);

    // lambda context is entered inside the visitor
    context().leave_lambda_expression();

    return true;
}

bool translation_unit_visitor::TraverseCallExpr(clang::CallExpr *expr)
{
    if (source_manager().isInSystemHeader(expr->getSourceRange().getBegin()))
        return true;

    LOG_TRACE("Entering call expression at {}",
        expr->getBeginLoc().printToString(source_manager()));

    context().enter_callexpr(expr);

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCallExpr(expr);

    LOG_TRACE("Leaving call expression at {}",
        expr->getBeginLoc().printToString(source_manager()));

    context().leave_callexpr();

    pop_message_to_diagram(expr);

    return true;
}

bool translation_unit_visitor::TraverseCUDAKernelCallExpr(
    clang::CUDAKernelCallExpr *expr)
{
    if (source_manager().isInSystemHeader(expr->getSourceRange().getBegin()))
        return true;

    LOG_TRACE("Entering CUDA kernel call expression at {}",
        expr->getBeginLoc().printToString(source_manager()));

    context().enter_callexpr(expr);

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCallExpr(expr);

    LOG_TRACE("Leaving CUDA kernel call expression at {}",
        expr->getBeginLoc().printToString(source_manager()));

    context().leave_callexpr();

    pop_message_to_diagram(expr);

    return true;
}

bool translation_unit_visitor::TraverseCXXMemberCallExpr(
    clang::CXXMemberCallExpr *expr)
{
    if (source_manager().isInSystemHeader(expr->getSourceRange().getBegin()))
        return true;

    LOG_TRACE("Entering member call expression at {}",
        expr->getBeginLoc().printToString(source_manager()));

    context().enter_callexpr(expr);

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXMemberCallExpr(
        expr);

    LOG_TRACE("Leaving member call expression at {}",
        expr->getBeginLoc().printToString(source_manager()));

    context().leave_callexpr();

    pop_message_to_diagram(expr);

    return true;
}

bool translation_unit_visitor::TraverseCXXOperatorCallExpr(
    clang::CXXOperatorCallExpr *expr)
{
    context().enter_callexpr(expr);

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXOperatorCallExpr(
        expr);

    context().leave_callexpr();

    pop_message_to_diagram(expr);

    return true;
}

bool translation_unit_visitor::TraverseCXXTemporaryObjectExpr(
    clang::CXXTemporaryObjectExpr *expr)
{
    context().enter_callexpr(expr);

    RecursiveASTVisitor<
        translation_unit_visitor>::TraverseCXXTemporaryObjectExpr(expr);

    translation_unit_visitor::VisitCXXConstructExpr(
        clang::dyn_cast<clang::CXXConstructExpr>(expr));

    context().leave_callexpr();

    pop_message_to_diagram(expr);

    return true;
}

bool translation_unit_visitor::TraverseCXXConstructExpr(
    clang::CXXConstructExpr *expr)
{
    LOG_TRACE("Entering cxx construct call expression at {}",
        expr->getBeginLoc().printToString(source_manager()));

    context().enter_callexpr(expr);

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXConstructExpr(
        expr);

    translation_unit_visitor::VisitCXXConstructExpr(expr);

    LOG_TRACE("Leaving cxx construct call expression at {}",
        expr->getBeginLoc().printToString(source_manager()));

    context().leave_callexpr();

    pop_message_to_diagram(expr);

    return true;
}

bool translation_unit_visitor::TraverseCompoundStmt(clang::CompoundStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::common::model::namespace_;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    if (stmt == nullptr)
        return true;

    const auto *current_ifstmt = context().current_ifstmt();
    const auto *current_elseifstmt =
        current_ifstmt != nullptr ? context().current_elseifstmt() : nullptr;

    //
    // Add final else block (not else if)
    //
    if (current_elseifstmt != nullptr) {
        if (current_elseifstmt->getElse() == stmt) {
            const auto current_caller_id = context().caller_id();

            if (current_caller_id.value() != 0) {
                model::message m{message_t::kElse, current_caller_id};
                set_source_location(*stmt, m);
                diagram().add_message(std::move(m));
            }
        }
    }
    else if (current_ifstmt != nullptr) {
        if (current_ifstmt->getElse() == stmt) {
            const auto current_caller_id = context().caller_id();

            if (current_caller_id.value() != 0) {
                model::message m{message_t::kElse, current_caller_id};
                set_source_location(*stmt, m);
                diagram().add_message(std::move(m));
            }
        }
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCompoundStmt(stmt);

    return true;
}

bool translation_unit_visitor::TraverseIfStmt(clang::IfStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::common::model::namespace_;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    bool elseif_block{false};

    const auto current_caller_id = context().caller_id();
    const auto *current_ifstmt = context().current_ifstmt();
    const auto *current_elseifstmt =
        current_ifstmt != nullptr ? context().current_elseifstmt() : nullptr;

    std::string condition_text;
    if (config().generate_condition_statements())
        condition_text = common::get_condition_text(source_manager(), stmt);

    // Check if this is a beginning of a new if statement, or an
    // else if condition of the current if statement
    auto child_stmt_compare = [stmt](auto *child_stmt) {
        return child_stmt == stmt;
    };

    if (current_ifstmt != nullptr)
        elseif_block = elseif_block ||
            std::any_of(current_ifstmt->children().begin(),
                current_ifstmt->children().end(), child_stmt_compare);
    if (current_elseifstmt != nullptr)
        elseif_block = elseif_block ||
            std::any_of(current_elseifstmt->children().begin(),
                current_elseifstmt->children().end(), child_stmt_compare);

    if ((current_caller_id.value() != 0) && !stmt->isConstexpr()) {
        if (elseif_block) {
            context().enter_elseifstmt(stmt);

            message m{message_t::kElseIf, current_caller_id};
            set_source_location(*stmt, m);
            m.condition_text(condition_text);
            m.set_comment(get_expression_comment(source_manager(),
                *context().get_ast_context(), current_caller_id, stmt));
            diagram().add_block_message(std::move(m));
        }
        else {
            context().enter_ifstmt(stmt);

            message m{message_t::kIf, current_caller_id};
            set_source_location(*stmt, m);
            m.condition_text(condition_text);
            m.set_comment(get_expression_comment(source_manager(),
                *context().get_ast_context(), current_caller_id, stmt));
            diagram().add_block_message(std::move(m));
        }
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseIfStmt(stmt);

    if ((current_caller_id.value() != 0) && !stmt->isConstexpr()) {
        if (!elseif_block) {
            diagram().end_block_message(
                {message_t::kIfEnd, current_caller_id}, message_t::kIf);
            context().leave_ifstmt();
        }
    }

    return true;
}

bool translation_unit_visitor::TraverseWhileStmt(clang::WhileStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    std::string condition_text;
    if (config().generate_condition_statements())
        condition_text = common::get_condition_text(source_manager(), stmt);

    if (current_caller_id.value() != 0) {
        context().enter_loopstmt(stmt);
        message m{message_t::kWhile, current_caller_id};
        set_source_location(*stmt, m);
        m.condition_text(condition_text);
        m.set_comment(get_expression_comment(source_manager(),
            *context().get_ast_context(), current_caller_id, stmt));
        diagram().add_block_message(std::move(m));
    }
    RecursiveASTVisitor<translation_unit_visitor>::TraverseWhileStmt(stmt);

    if (current_caller_id.value() != 0) {
        diagram().end_block_message(
            {message_t::kWhileEnd, current_caller_id}, message_t::kWhile);
        context().leave_loopstmt();
    }

    return true;
}

bool translation_unit_visitor::TraverseDoStmt(clang::DoStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    std::string condition_text;
    if (config().generate_condition_statements())
        condition_text = common::get_condition_text(source_manager(), stmt);

    if (current_caller_id.value() != 0) {
        context().enter_loopstmt(stmt);
        message m{message_t::kDo, current_caller_id};
        set_source_location(*stmt, m);
        m.condition_text(condition_text);
        m.set_comment(get_expression_comment(source_manager(),
            *context().get_ast_context(), current_caller_id, stmt));
        diagram().add_block_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseDoStmt(stmt);

    if (current_caller_id.value() != 0) {
        diagram().end_block_message(
            {message_t::kDoEnd, current_caller_id}, message_t::kDo);
        context().leave_loopstmt();
    }

    return true;
}

bool translation_unit_visitor::TraverseForStmt(clang::ForStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    std::string condition_text;
    if (config().generate_condition_statements())
        condition_text = common::get_condition_text(source_manager(), stmt);

    if (current_caller_id.value() != 0) {
        context().enter_loopstmt(stmt);
        message m{message_t::kFor, current_caller_id};
        set_source_location(*stmt, m);
        m.condition_text(condition_text);

        m.set_comment(get_expression_comment(source_manager(),
            *context().get_ast_context(), current_caller_id, stmt));

        diagram().add_block_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseForStmt(stmt);

    if (current_caller_id.value() != 0) {
        diagram().end_block_message(
            {message_t::kForEnd, current_caller_id}, message_t::kFor);
        context().leave_loopstmt();
    }

    return true;
}

bool translation_unit_visitor::TraverseCXXTryStmt(clang::CXXTryStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    if (current_caller_id.value() != 0) {
        context().enter_trystmt(stmt);
        message m{message_t::kTry, current_caller_id};
        set_source_location(*stmt, m);
        m.set_comment(get_expression_comment(source_manager(),
            *context().get_ast_context(), current_caller_id, stmt));
        diagram().add_block_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXTryStmt(stmt);

    if (current_caller_id.value() != 0) {
        diagram().end_block_message(
            {message_t::kTryEnd, current_caller_id}, message_t::kTry);
        context().leave_trystmt();
    }

    return true;
}

bool translation_unit_visitor::TraverseCXXCatchStmt(clang::CXXCatchStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    if ((current_caller_id.value() != 0) &&
        (context().current_trystmt() != nullptr)) {
        std::string caught_type;
        if (stmt->getCaughtType().isNull())
            caught_type = "...";
        else
            caught_type = common::to_string(
                stmt->getCaughtType(), *context().get_ast_context());

        model::message m{message_t::kCatch, current_caller_id};
        m.set_message_name(std::move(caught_type));
        diagram().add_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXCatchStmt(stmt);

    return true;
}

bool translation_unit_visitor::TraverseCXXForRangeStmt(
    clang::CXXForRangeStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    std::string condition_text;
    if (config().generate_condition_statements())
        condition_text = common::get_condition_text(source_manager(), stmt);

    if (current_caller_id.value() != 0) {
        context().enter_loopstmt(stmt);
        message m{message_t::kFor, current_caller_id};
        set_source_location(*stmt, m);
        m.condition_text(condition_text);
        m.set_comment(get_expression_comment(source_manager(),
            *context().get_ast_context(), current_caller_id, stmt));
        diagram().add_block_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXForRangeStmt(
        stmt);

    if (current_caller_id.value() != 0) {
        diagram().end_block_message(
            {message_t::kForEnd, current_caller_id}, message_t::kFor);
        context().leave_loopstmt();
    }

    return true;
}

bool translation_unit_visitor::TraverseSwitchStmt(clang::SwitchStmt *stmt)
{
    using clanguml::common::model::message_t;

    const auto current_caller_id = context().caller_id();

    if (current_caller_id.value() != 0) {
        context().enter_switchstmt(stmt);
        model::message m{message_t::kSwitch, current_caller_id};
        set_source_location(*stmt, m);
        m.set_comment(get_expression_comment(source_manager(),
            *context().get_ast_context(), current_caller_id, stmt));
        diagram().add_block_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseSwitchStmt(stmt);

    if (current_caller_id.value() != 0) {
        context().leave_switchstmt();
        diagram().end_block_message(
            {message_t::kSwitchEnd, current_caller_id}, message_t::kSwitch);
    }

    return true;
}

bool translation_unit_visitor::TraverseCaseStmt(clang::CaseStmt *stmt)
{
    using clanguml::common::model::message_t;

    const auto current_caller_id = context().caller_id();

    if ((current_caller_id.value() != 0) &&
        (context().current_switchstmt() != nullptr)) {
        model::message m{message_t::kCase, current_caller_id};
        m.set_message_name(common::to_string(stmt->getLHS()));
        diagram().add_case_stmt_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCaseStmt(stmt);

    return true;
}

bool translation_unit_visitor::TraverseDefaultStmt(clang::DefaultStmt *stmt)
{
    using clanguml::common::model::message_t;

    const auto current_caller_id = context().caller_id();

    if ((current_caller_id.value() != 0) &&
        (context().current_switchstmt() != nullptr)) {
        model::message m{message_t::kCase, current_caller_id};
        m.set_message_name("default");
        diagram().add_case_stmt_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseDefaultStmt(stmt);

    return true;
}

bool translation_unit_visitor::TraverseConditionalOperator(
    clang::ConditionalOperator *stmt)
{
    using clanguml::common::model::message_t;

    const auto current_caller_id = context().caller_id();

    std::string condition_text;
    if (config().generate_condition_statements())
        condition_text = common::get_condition_text(source_manager(), stmt);

    if (current_caller_id.value() != 0) {
        context().enter_conditionaloperator(stmt);
        model::message m{message_t::kConditional, current_caller_id};
        set_source_location(*stmt, m);
        m.condition_text(condition_text);
        m.set_comment(get_expression_comment(source_manager(),
            *context().get_ast_context(), current_caller_id, stmt));
        diagram().add_block_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseStmt(
        stmt->getCond());

    RecursiveASTVisitor<translation_unit_visitor>::TraverseStmt(
        stmt->getTrueExpr());

    if (current_caller_id.value() != 0) {
        model::message m{message_t::kConditionalElse, current_caller_id};
        set_source_location(*stmt, m);
        diagram().add_message(std::move(m));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseStmt(
        stmt->getFalseExpr());

    if (current_caller_id.value() != 0) {
        context().leave_conditionaloperator();
        diagram().end_block_message(
            {message_t::kConditionalEnd, current_caller_id},
            message_t::kConditional);
    }

    return true;
}

bool translation_unit_visitor::VisitCallExpr(clang::CallExpr *expr)
{
    using clanguml::common::model::message_scope_t;
    using clanguml::common::model::message_t;
    using clanguml::common::model::namespace_;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    if (!context().valid() || context().get_ast_context() == nullptr)
        return true;

    LOG_TRACE("Visiting call expression at {} [caller_id = {}]",
        expr->getBeginLoc().printToString(source_manager()),
        context().caller_id());

    message m{message_t::kCall, context().caller_id()};

    m.in_static_declaration_context(within_static_variable_declaration_ > 0);

    set_source_location(*expr, m);

    const auto *raw_expr_comment = clanguml::common::get_expression_raw_comment(
        source_manager(), *context().get_ast_context(), expr);
    const auto stripped_comment = process_comment(
        raw_expr_comment, context().get_ast_context()->getDiagnostics(), m);

    if (m.skip())
        return true;

    auto generated_message_from_comment = generate_message_from_comment(m);

    if (!generated_message_from_comment && !should_include(expr)) {
        processed_comments().erase(raw_expr_comment);
        return true;
    }

    if (context().is_expr_in_current_control_statement_condition(expr)) {
        m.set_message_scope(common::model::message_scope_t::kCondition);
    }

    if (generated_message_from_comment) {
        LOG_DBG(
            "Message for this call expression is taken from comment directive");
    }
    //
    // Call to a CUDA kernel function
    //
    else if (const auto *cuda_call_expr =
                 clang::dyn_cast_or_null<clang::CUDAKernelCallExpr>(expr);
             cuda_call_expr != nullptr) {
        if (!process_cuda_kernel_call_expression(m, cuda_call_expr))
            return true;
    }
    //
    // Call to an overloaded operator
    //
    else if (const auto *operator_call_expr =
                 clang::dyn_cast_or_null<clang::CXXOperatorCallExpr>(expr);
             operator_call_expr != nullptr) {

        if (!process_operator_call_expression(m, operator_call_expr))
            return true;
    }
    //
    // Call to a class method
    //
    else if (const auto *method_call_expr =
                 clang::dyn_cast_or_null<clang::CXXMemberCallExpr>(expr);
             method_call_expr != nullptr) {

        if (!process_class_method_call_expression(m, method_call_expr))
            return true;
    }
    //
    // Call to function or template
    //
    else {
        auto *callee_decl = expr->getCalleeDecl();

        if (callee_decl == nullptr) {
            LOG_DBG("Cannot get callee declaration - trying direct function "
                    "callee...");

            callee_decl = expr->getDirectCallee();

            if (callee_decl != nullptr)
                LOG_DBG("Found function/method callee in: {}",
                    common::to_string(expr));
        }

        if (callee_decl == nullptr) {
            //
            // Call to a method of a class template
            //
            if (clang::dyn_cast_or_null<clang::CXXDependentScopeMemberExpr>(
                    expr->getCallee()) != nullptr) {
                if (!process_class_template_method_call_expression(m, expr)) {
                    return true;
                }
            }
            //
            // Unresolved lookup expression are sometimes calls to template
            // functions
            //
            else if (clang::dyn_cast_or_null<clang::UnresolvedLookupExpr>(
                         expr->getCallee()) != nullptr) {
                if (!process_unresolved_lookup_call_expression(m, expr))
                    return true;
            }
            else if (clang::dyn_cast_or_null<clang::LambdaExpr>(
                         expr->getCallee()) != nullptr) {
                LOG_DBG("Processing lambda expression callee");
                if (!process_lambda_call_expression(m, expr))
                    return true;
            }
            else {
                LOG_DBG("Found unsupported callee decl type for: {} at {}",
                    common::to_string(expr),
                    expr->getBeginLoc().printToString(source_manager()));
            }
        }
        else {
            auto success = process_function_call_expression(m, expr);

            if (!success) {
                LOG_DBG("Skipping call to call expression at: {}",
                    expr->getBeginLoc().printToString(source_manager()));

                return true;
            }
        }
    }

    // Add message to diagram
    if (m.from().value() > 0 && m.to().value() > 0) {
        m.set_comment(stripped_comment);

        if (diagram().sequences().find(m.from()) ==
            diagram().sequences().end()) {
            activity a{m.from()};
            diagram().sequences().insert({m.from(), std::move(a)});
        }

        diagram().add_active_participant(m.from());
        diagram().add_active_participant(m.to());

        LOG_DBG("Found call {} from {} [{}] to {} [{}] ", m.message_name(),
            m.from(), m.from(), m.to(), m.to());

        push_message(expr, std::move(m));
    }

    return true;
}

bool translation_unit_visitor::generate_message_from_comment(
    model::message &m) const
{
    auto generated_message_from_comment{false};
    for (const auto &decorator : m.decorators()) {
        auto call_decorator =
            std::dynamic_pointer_cast<decorators::call>(decorator);
        if (call_decorator &&
            call_decorator->applies_to_diagram(config().name)) {
            m.set_to(common::to_id(call_decorator->callee));
            generated_message_from_comment = true;
            break;
        }
    }
    return generated_message_from_comment;
}

bool translation_unit_visitor::TraverseVarDecl(clang::VarDecl *decl)
{
    if (decl->isStaticLocal())
        within_static_variable_declaration_++;

    RecursiveASTVisitor::TraverseVarDecl(decl);

    if (decl->isStaticLocal())
        within_static_variable_declaration_--;

    return true;
}

bool translation_unit_visitor::VisitCXXConstructExpr(
    clang::CXXConstructExpr *expr)
{
    using clanguml::common::model::message_scope_t;
    using clanguml::common::model::message_t;
    using clanguml::common::model::namespace_;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    if (expr == nullptr)
        return true;

    if (const auto *ctor = expr->getConstructor();
        ctor != nullptr && !should_include(ctor))
        return true;

    LOG_TRACE("Visiting cxx construct expression at {} [caller_id = {}]",
        expr->getBeginLoc().printToString(source_manager()),
        context().caller_id());

    message m{message_t::kCall, context().caller_id()};

    m.in_static_declaration_context(within_static_variable_declaration_ > 0);

    set_source_location(*expr, m);

    if (context().is_expr_in_current_control_statement_condition(expr)) {
        m.set_message_scope(common::model::message_scope_t::kCondition);
    }

    if (!process_construct_expression(m, expr))
        return true;

    if (m.from().value() > 0 && m.to().value() > 0) {
        if (diagram().sequences().find(m.from()) ==
            diagram().sequences().end()) {
            activity a{m.from()};
            diagram().sequences().insert({m.from(), std::move(a)});
        }

        diagram().add_active_participant(m.from());
        diagram().add_active_participant(m.to());

        LOG_DBG("Found constructor call {} from {} [{}] to {} [{}] ",
            m.message_name(), m.from(), m.from(), m.to(), m.to());

        push_message(expr, std::move(m));
    }

    return true;
}

bool translation_unit_visitor::process_cuda_kernel_call_expression(
    model::message &m, const clang::CUDAKernelCallExpr *expr)
{
    const auto *callee_decl = expr->getCalleeDecl();

    if (callee_decl == nullptr)
        return false;

    const auto *callee_function = callee_decl->getAsFunction();

    if (callee_function == nullptr)
        return false;

    if (!should_include(callee_function))
        return false;

    // Skip free functions declared in files outside of included paths
    if (config().combine_free_functions_into_file_participants() &&
        !diagram().should_include(common::model::source_file{m.file()}))
        return false;

    auto callee_name = callee_function->getQualifiedNameAsString() + "()";

    const auto maybe_id = get_unique_id(eid_t{callee_function->getID()});
    if (!maybe_id.has_value()) {
        // This is hopefully not an interesting call...
        m.set_to(eid_t{callee_function->getID()});
    }
    else {
        m.set_to(maybe_id.value());
    }

    m.set_message_name(callee_name.substr(0, callee_name.size() - 2));

    return true;
}

bool translation_unit_visitor::process_operator_call_expression(
    model::message &m, const clang::CXXOperatorCallExpr *operator_call_expr)
{
    if (operator_call_expr->getCalleeDecl() == nullptr)
        return false;

    LOG_DBG("Operator '{}' call expression to {} at {}",
        getOperatorSpelling(operator_call_expr->getOperator()),
        operator_call_expr->getCalleeDecl()->getID(),
        operator_call_expr->getBeginLoc().printToString(source_manager()));

    // Handle the case if the callee is a lambda
    if (const auto *lambda_method = clang::dyn_cast<clang::CXXMethodDecl>(
            operator_call_expr->getCalleeDecl());
        lambda_method != nullptr && lambda_method->getParent()->isLambda()) {

        LOG_DBG("Operator callee is a lambda: {}",
            common::to_string(lambda_method));

        const auto source_location{
            lambda_source_location(lambda_method->getParent()->getLocation())};

        auto lambda_name = make_lambda_name(lambda_method->getParent());

        m.set_to(eid_t{lambda_method->getParent()->getID()});
    }
    else {
        auto maybe_id =
            get_unique_id(eid_t{operator_call_expr->getCalleeDecl()->getID()});
        if (maybe_id.has_value()) {
            m.set_to(maybe_id.value());
        }
        else {
            m.set_to(eid_t{operator_call_expr->getCalleeDecl()->getID()});
        }
    }

    m.set_message_name(fmt::format(
        "operator{}", getOperatorSpelling(operator_call_expr->getOperator())));

    return true;
}

bool translation_unit_visitor::process_construct_expression(
    model::message &m, const clang::CXXConstructExpr *construct_expr)
{
    const auto *constructor = construct_expr->getConstructor();
    if (constructor == nullptr)
        return false;

    const auto *constructor_parent = constructor->getParent();
    if (constructor_parent == nullptr)
        return false;

    LOG_DBG("Constructor '{}' call expression to {} at {}",
        construct_expr->getConstructor()->getNameAsString(),
        constructor->getID(),
        construct_expr->getBeginLoc().printToString(source_manager()));

    auto maybe_id = get_unique_id(eid_t{constructor->getID()});
    if (maybe_id.has_value()) {
        m.set_to(maybe_id.value());
    }
    else {
        m.set_to(eid_t{constructor->getID()});
    }

    m.set_message_name(
        fmt::format("{}::{}", constructor_parent->getQualifiedNameAsString(),
            constructor_parent->getNameAsString()));

    diagram().add_active_participant(eid_t{constructor->getID()});

    return true;
}

bool translation_unit_visitor::process_class_method_call_expression(
    model::message &m, const clang::CXXMemberCallExpr *method_call_expr)
{
    // Get callee declaration as methods parent
    const auto *method_decl = method_call_expr->getMethodDecl();

    if (method_decl == nullptr)
        return false;

    std::string method_name = method_decl->getQualifiedNameAsString();

    const auto *callee_decl =
        method_decl != nullptr ? method_decl->getParent() : nullptr;

    if (callee_decl == nullptr)
        return false;

    if (!should_include(callee_decl) || !should_include(method_decl))
        return false;

    m.set_to(eid_t{method_decl->getID()});
    m.set_message_name(method_decl->getNameAsString());
    m.set_return_type(
        method_call_expr->getCallReturnType(*context().get_ast_context())
            .getAsString());

    LOG_TRACE("Set callee method id {} for method name {}", m.to(),
        method_decl->getQualifiedNameAsString());

    diagram().add_active_participant(eid_t{method_decl->getID()});

    return true;
}

bool translation_unit_visitor::process_class_template_method_call_expression(
    model::message &m, const clang::CallExpr *expr)
{
    const auto *dependent_member_callee =
        clang::dyn_cast_or_null<clang::CXXDependentScopeMemberExpr>(
            expr->getCallee());

    if (dependent_member_callee == nullptr)
        return false;

    if (is_callee_valid_template_specialization(dependent_member_callee)) {
        if (const auto *tst = dependent_member_callee->getBaseType()
                                  ->getAs<clang::TemplateSpecializationType>();
            tst != nullptr) {
            const auto *template_declaration =
                tst->getTemplateName().getAsTemplateDecl();

            std::string callee_method_full_name;

            // First check if the primary template is already in the
            // participants map
            if (get_participant(template_declaration).has_value()) {
                callee_method_full_name = get_participant(template_declaration)
                                              .value()
                                              .full_name(false) +
                    "::" + dependent_member_callee->getMember().getAsString();

                for (const auto &[id, p] : diagram().participants()) {
                    const auto p_full_name = p->full_name(false);

                    if (p_full_name.find(callee_method_full_name + "(") == 0) {
                        // TODO: This selects the first matching template method
                        //       without considering arguments!!!
                        m.set_to(id);
                        break;
                    }
                }
            }
            // Otherwise check if it is a smart pointer
            else if (is_smart_pointer(template_declaration)) {
                const auto *argument_template =
                    template_declaration->getTemplateParameters()
                        ->asArray()
                        .front();

                if (get_participant(argument_template).has_value()) {
                    callee_method_full_name = get_participant(argument_template)
                                                  .value()
                                                  .full_name(false) +
                        "::" +
                        dependent_member_callee->getMember().getAsString();

                    for (const auto &[id, p] : diagram().participants()) {
                        const auto p_full_name = p->full_name(false);
                        if (p_full_name.find(callee_method_full_name + "(") ==
                            0) {
                            // TODO: This selects the first matching template
                            // method
                            //       without considering arguments!!!
                            m.set_to(id);
                            break;
                        }
                    }
                }
                else
                    return false;
            }

            m.set_message_name(
                dependent_member_callee->getMember().getAsString());

            if (const auto maybe_id =
                    get_unique_id(eid_t{template_declaration->getID()});
                maybe_id.has_value())
                diagram().add_active_participant(maybe_id.value());
        }
    }
    else {
        LOG_DBG("Skipping call due to unresolvable "
                "CXXDependentScopeMemberExpr at {}",
            expr->getBeginLoc().printToString(source_manager()));
    }

    return true;
}

bool translation_unit_visitor::process_function_call_expression(
    model::message &m, const clang::CallExpr *expr)
{
    const auto *callee_decl = expr->getCalleeDecl();

    if (callee_decl == nullptr)
        return false;

    const auto *callee_function = callee_decl->getAsFunction();

    if (callee_function == nullptr)
        return false;

    if (!should_include(callee_function))
        return false;

    // Skip free functions declared in files outside of included paths
    if (config().combine_free_functions_into_file_participants() &&
        !diagram().should_include(common::model::source_file{m.file()}))
        return false;

    auto callee_name = callee_function->getQualifiedNameAsString() + "()";

    const auto maybe_id = get_unique_id(eid_t{callee_function->getID()});
    if (!maybe_id.has_value()) {
        // This is hopefully not an interesting call...
        m.set_to(eid_t{callee_function->getID()});
    }
    else {
        m.set_to(maybe_id.value());
    }

    m.set_message_name(callee_name.substr(0, callee_name.size() - 2));

    return true;
}

bool translation_unit_visitor::process_lambda_call_expression(
    model::message &m, const clang::CallExpr *expr) const
{
    const auto *lambda_expr =
        clang::dyn_cast_or_null<clang::LambdaExpr>(expr->getCallee());

    if (lambda_expr == nullptr)
        return true;

    const auto lambda_class_id = eid_t{lambda_expr->getLambdaClass()->getID()};
    const auto maybe_id = get_unique_id(lambda_class_id);
    if (!maybe_id.has_value())
        m.set_to(lambda_class_id);
    else {
        m.set_to(maybe_id.value());
    }

    return true;
}

bool translation_unit_visitor::process_unresolved_lookup_call_expression(
    model::message &m, const clang::CallExpr *expr) const
{
    // This is probably a template
    const auto *unresolved_expr =
        clang::dyn_cast_or_null<clang::UnresolvedLookupExpr>(expr->getCallee());

    if (unresolved_expr != nullptr) {
        for (const auto *decl : unresolved_expr->decls()) {
            if (clang::dyn_cast_or_null<clang::FunctionTemplateDecl>(decl) !=
                nullptr) {
                const auto *ftd =
                    clang::dyn_cast_or_null<clang::FunctionTemplateDecl>(decl);

                const auto maybe_id = get_unique_id(eid_t{ftd->getID()});
                if (!maybe_id.has_value())
                    m.set_to(eid_t{ftd->getID()});
                else {
                    m.set_to(maybe_id.value());
                }

                break;
            }

            if (clang::dyn_cast_or_null<clang::FunctionDecl>(decl) != nullptr) {
                const auto *fd =
                    clang::dyn_cast_or_null<clang::FunctionDecl>(decl);

                const auto maybe_id = get_unique_id(eid_t{fd->getID()});
                if (!maybe_id.has_value())
                    m.set_to(eid_t{fd->getID()});
                else {
                    m.set_to(maybe_id.value());
                }

                break;
            }

            LOG_DBG("Unknown unresolved lookup expression");
        }
    }

    return true;
}

bool translation_unit_visitor::is_callee_valid_template_specialization(
    const clang::CXXDependentScopeMemberExpr *dependent_member_expr) const
{
    if (dependent_member_expr == nullptr)
        return false;

    if (dependent_member_expr->getBaseType().isNull())
        return false;

    const auto *tst = dependent_member_expr->getBaseType()
                          ->getAs<clang::TemplateSpecializationType>();

    if (tst == nullptr)
        return false;

    return !(tst->isPointerType());
}

bool translation_unit_visitor::is_smart_pointer(
    const clang::TemplateDecl *primary_template) const
{
    return primary_template->getQualifiedNameAsString().find(
               "std::unique_ptr") == 0 ||
        primary_template->getQualifiedNameAsString().find("std::shared_ptr") ==
        0 ||
        primary_template->getQualifiedNameAsString().find("std::weak_ptr") == 0;
}

std::unique_ptr<clanguml::sequence_diagram::model::class_>
translation_unit_visitor::create_class_model(clang::CXXRecordDecl *cls)
{
    assert(cls != nullptr);

    auto c_ptr{std::make_unique<clanguml::sequence_diagram::model::class_>(
        config().using_namespace())};
    auto &c = *c_ptr;

    auto qualified_name = cls->getQualifiedNameAsString();

    if (!cls->isLambda())
        if (!should_include(cls))
            return {};

    auto ns = common::get_tag_namespace(*cls);

    if (cls->isLambda() && !diagram().should_include(ns | "lambda"))
        return {};

    const auto *parent = cls->getParent();

    if ((parent != nullptr) && parent->isRecord()) {
        // Here we have 3 options, either:
        //  - the parent is a regular C++ class/struct
        //  - the parent is a class template declaration/specialization
        //  - the parent is a lambda (i.e. this is a nested lambda expression)
        std::optional<eid_t> id_opt;
        const auto *parent_record_decl =
            clang::dyn_cast<clang::RecordDecl>(parent);

        assert(parent_record_decl != nullptr);

        const eid_t ast_id{parent_record_decl->getID()};

        // First check if the parent has been added to the diagram as
        // regular class
        id_opt = get_unique_id(ast_id);

        // If not, check if the parent template declaration is in the model
        if (!id_opt &&
            (parent_record_decl->getDescribedTemplate() != nullptr)) {
            parent_record_decl->getDescribedTemplate()->getID();
            if (parent_record_decl->getDescribedTemplate() != nullptr)
                id_opt = get_unique_id(ast_id);
        }

        if (!id_opt)
            return {};

        auto parent_class =
            diagram()
                .get_participant<clanguml::sequence_diagram::model::class_>(
                    *id_opt);

        assert(parent_class);

        c.set_namespace(ns);
        if (cls->getNameAsString().empty()) {
            // Nested structs can be anonymous
            if (anonymous_struct_relationships_.count(cls->getID()) > 0) {
                const auto &[label, hint, access] =
                    anonymous_struct_relationships_[cls->getID()];

                c.set_name(parent_class.value().name() +
                    "::" + fmt::format("({})", label));

                parent_class.value().add_relationship(
                    {hint, common::to_id(c.full_name(false)), access, label});
            }
            else
                c.set_name(parent_class.value().name() + "::" +
                    fmt::format(
                        "(anonymous_{})", std::to_string(cls->getID())));
        }
        else {
            c.set_name(
                parent_class.value().name() + "::" + cls->getNameAsString());
        }

        c.set_id(common::to_id(c.full_name(false)));

        c.nested(true);
    }
    else if (cls->isLambda()) {
        c.is_lambda(true);
        if (cls->getParent() != nullptr) {
            const auto type_name = make_lambda_name(cls);

            c.set_name(type_name);
            c.set_namespace(ns);
            c.set_id(common::to_id(c.full_name(false)));
        }
        else {
            LOG_WARN("Cannot find parent declaration for lambda {}",
                cls->getQualifiedNameAsString());
            return {};
        }
    }
    else {
        c.set_name(common::get_tag_name(*cls));
        c.set_namespace(ns);
        c.set_id(common::to_id(c.full_name(false)));
    }

    c.is_struct(cls->isStruct());

    process_comment(*cls, c);
    set_source_location(*cls, c);

    if (c.skip())
        return {};

    c.set_style(c.style_spec());

    return c_ptr;
}

void translation_unit_visitor::set_unique_id(int64_t local_id, eid_t global_id)
{
    LOG_TRACE("Setting local element mapping {} --> {}", local_id, global_id);

    local_ast_id_map_[local_id] = global_id;
}

std::optional<eid_t> translation_unit_visitor::get_unique_id(
    eid_t local_id) const
{
    if (local_ast_id_map_.find(local_id.ast_local_value()) ==
        local_ast_id_map_.end())
        return {};

    return local_ast_id_map_.at(local_id.ast_local_value());
}

std::unique_ptr<model::function_template>
translation_unit_visitor::build_function_template(
    const clang::FunctionTemplateDecl &declaration)
{
    auto function_template_model_ptr =
        std::make_unique<sequence_diagram::model::function_template>(
            config().using_namespace());

    set_qualified_name(declaration, *function_template_model_ptr);

    tbuilder().build_from_template_declaration(
        *function_template_model_ptr, declaration);

    function_template_model_ptr->return_type(
        common::to_string(declaration.getAsFunction()->getReturnType(),
            declaration.getASTContext()));

    for (const auto *param : declaration.getTemplatedDecl()->parameters()) {
        function_template_model_ptr->add_parameter(
            simplify_system_template(common::to_string(
                param->getType(), declaration.getASTContext(), false)));
    }

    return function_template_model_ptr;
}

std::unique_ptr<model::function_template>
translation_unit_visitor::build_function_template_instantiation(
    const clang::FunctionDecl &decl)
{
    auto template_instantiation_ptr =
        std::make_unique<model::function_template>(config().using_namespace());
    auto &template_instantiation = *template_instantiation_ptr;

    set_qualified_name(decl, template_instantiation);

    tbuilder().build(template_instantiation, &decl, decl.getPrimaryTemplate(),
        decl.getTemplateSpecializationArgs()->asArray(),
        common::to_string(&decl));

    // Handle function parameters
    for (const auto *param : decl.parameters()) {
        template_instantiation_ptr->add_parameter(
            common::to_string(param->getType(), decl.getASTContext()));
    }

    return template_instantiation_ptr;
}

std::unique_ptr<model::function> translation_unit_visitor::build_function_model(
    const clang::FunctionDecl &declaration)
{
    auto function_model_ptr =
        std::make_unique<sequence_diagram::model::function>(
            config().using_namespace());

    common::model::namespace_ ns{declaration.getQualifiedNameAsString()};
    function_model_ptr->set_name(ns.name());
    ns.pop_back();
    function_model_ptr->set_namespace(ns);

    function_model_ptr->return_type(common::to_string(
        declaration.getReturnType(), declaration.getASTContext()));

    for (const auto *param : declaration.parameters()) {
        function_model_ptr->add_parameter(
            simplify_system_template(common::to_string(
                param->getType(), declaration.getASTContext(), false)));
    }

    return function_model_ptr;
}

std::unique_ptr<model::class_>
translation_unit_visitor::process_class_template_specialization(
    clang::ClassTemplateSpecializationDecl *cls)
{
    auto c_ptr{std::make_unique<model::class_>(config().using_namespace())};

    tbuilder().build_from_class_template_specialization(*c_ptr, *cls);

    auto &template_instantiation = *c_ptr;

    // TODO: refactor to method get_qualified_name()
    auto qualified_name = cls->getQualifiedNameAsString();
    util::replace_all(qualified_name, "(anonymous namespace)", "");
    util::replace_all(qualified_name, "::::", "::");

    common::model::namespace_ ns{qualified_name};
    ns.pop_back();
    template_instantiation.set_name(cls->getNameAsString());
    template_instantiation.set_namespace(ns);

    template_instantiation.is_struct(cls->isStruct());

    process_comment(*cls, template_instantiation);
    set_source_location(*cls, template_instantiation);
    set_owning_module(*cls, template_instantiation);

    if (template_instantiation.skip())
        return {};

    template_instantiation.set_id(
        common::to_id(template_instantiation.full_name(false)));

    set_unique_id(cls->getID(), template_instantiation.id());

    return c_ptr;
}

std::string translation_unit_visitor::simplify_system_template(
    const std::string &full_name) const
{
    return config().simplify_template_type(full_name);
}

std::string translation_unit_visitor::lambda_source_location(
    const clang::SourceLocation &source_location) const
{
    const auto file_line =
        source_manager().getSpellingLineNumber(source_location);
    const auto file_column =
        source_manager().getSpellingColumnNumber(source_location);
    const std::string file_name =
        config()
            .make_path_relative(
                source_manager().getFilename(source_location).str())
            .string();
    return fmt::format("{}:{}:{}", file_name, file_line, file_column);
}

std::string translation_unit_visitor::make_lambda_name(
    const clang::CXXRecordDecl *cls) const
{
    std::string result;
    const auto location = cls->getLocation();
    const std::string source_location{lambda_source_location(location)};

    const auto maybe_lambda_caller_id = context().lambda_caller_id();
    if (maybe_lambda_caller_id.has_value()) {
        // Parent is also a lambda (this id points to a lambda operator())
        std::string parent_lambda_class_name{"()"};
        if (diagram().get_participant<model::method>(
                maybe_lambda_caller_id.value())) {
            auto parent_lambda_class_id =
                diagram()
                    .get_participant<model::method>(
                        maybe_lambda_caller_id.value())
                    .value()
                    .class_id();

            if (diagram().get_participant<model::class_>(
                    parent_lambda_class_id)) {
                parent_lambda_class_name =
                    diagram()
                        .get_participant<model::class_>(parent_lambda_class_id)
                        .value()
                        .full_name(false);
            }
        }

        result = fmt::format(
            "{}##(lambda {})", parent_lambda_class_name, source_location);
    }
    else if (context().caller_id().value() != 0 &&
        get_participant(context().caller_id()).has_value()) {
        auto parent_full_name =
            get_participant(context().caller_id()).value().full_name_no_ns();

        result =
            fmt::format("{}##(lambda {})", parent_full_name, source_location);
    }
    else {
        result = fmt::format("(lambda {})", source_location);
    }

    return result;
}

void translation_unit_visitor::push_message(
    clang::CallExpr *expr, model::message &&m)
{
    call_expr_message_map_.emplace(expr, std::move(m));
}

void translation_unit_visitor::push_message(
    clang::CXXConstructExpr *expr, model::message &&m)
{
    construct_expr_message_map_.emplace(expr, std::move(m));
}

void translation_unit_visitor::pop_message_to_diagram(clang::CallExpr *expr)
{
    assert(expr != nullptr);

    // Skip if no message was generated from this expr
    if (call_expr_message_map_.find(expr) == call_expr_message_map_.end()) {
        return;
    }

    auto msg = std::move(call_expr_message_map_.at(expr));

    auto caller_id = msg.from();

    if (caller_id == 0)
        return;

    if (diagram().has_activity(caller_id))
        diagram().get_activity(caller_id).add_message(std::move(msg));

    call_expr_message_map_.erase(expr);
}

void translation_unit_visitor::pop_message_to_diagram(
    clang::CXXConstructExpr *expr)
{
    assert(expr != nullptr);

    // Skip if no message was generated from this expr
    if (construct_expr_message_map_.find(expr) ==
        construct_expr_message_map_.end()) {
        return;
    }

    auto msg = std::move(construct_expr_message_map_.at(expr));

    auto caller_id = msg.from();
    diagram().get_activity(caller_id).add_message(std::move(msg));

    construct_expr_message_map_.erase(expr);
}

void translation_unit_visitor::finalize()
{
    resolve_ids_to_global();

    // Change all messages with target set to an id of a lambda expression to
    // to the ID of their operator() - this is necessary, as some calls to
    // lambda expressions are visited before the actual lambda expressions
    // are visited...
    ensure_lambda_messages_have_operator_as_target();

    if (config().inline_lambda_messages())
        diagram().inline_lambda_operator_calls();
}

void translation_unit_visitor::ensure_lambda_messages_have_operator_as_target()
{
    for (auto &[id, activity] : diagram().sequences()) {
        for (auto &m : activity.messages()) {
            auto participant = diagram().get_participant<model::class_>(m.to());

            if (participant && participant.value().is_lambda() &&
                participant.value().lambda_operator_id().value() != 0) {
                LOG_DBG("Changing lambda expression target id from {} to {}",
                    m.to(), participant.value().lambda_operator_id());

                m.set_to(participant.value().lambda_operator_id());
                m.set_message_name("operator()");
            }
        }
    }
}

void translation_unit_visitor::resolve_ids_to_global()
{
    std::set<eid_t> active_participants_unique;

    // Change all active participants AST local ids to diagram global ids
    for (auto id : diagram().active_participants()) {
        if (!id.is_global() &&
            local_ast_id_map_.find(id.ast_local_value()) !=
                local_ast_id_map_.end()) {
            active_participants_unique.emplace(
                local_ast_id_map_.at(id.ast_local_value()));
        }
        else if (id.is_global()) {
            active_participants_unique.emplace(id);
        }
    }

    diagram().active_participants() = std::move(active_participants_unique);

    // Change all message callees AST local ids to diagram global ids
    for (auto &[id, activity] : diagram().sequences()) {
        for (auto &m : activity.messages()) {
            if (!m.to().is_global() &&
                local_ast_id_map_.find(m.to().ast_local_value()) !=
                    local_ast_id_map_.end()) {
                m.set_to(local_ast_id_map_.at(m.to().ast_local_value()));
            }
        }
    }
}

std::unique_ptr<clanguml::sequence_diagram::model::method>
translation_unit_visitor::create_lambda_method_model(
    clang::CXXMethodDecl *declaration)
{
    auto method_model_ptr = std::make_unique<sequence_diagram::model::method>(
        config().using_namespace());

    common::model::namespace_ ns{declaration->getQualifiedNameAsString()};
    auto method_name = ns.name();
    method_model_ptr->set_method_name(method_name);
    ns.pop_back();
    method_model_ptr->set_name(ns.name());
    ns.pop_back();

    method_model_ptr->is_defaulted(declaration->isDefaulted());
    method_model_ptr->is_assignment(declaration->isCopyAssignmentOperator() ||
        declaration->isMoveAssignmentOperator());
    method_model_ptr->is_const(declaration->isConst());
    method_model_ptr->is_static(declaration->isStatic());
    method_model_ptr->is_operator(declaration->isOverloadedOperator());
    method_model_ptr->is_constructor(
        clang::dyn_cast<clang::CXXConstructorDecl>(declaration) != nullptr);

    method_model_ptr->is_void(declaration->getReturnType()->isVoidType());

    method_model_ptr->return_type(common::to_string(
        declaration->getReturnType(), declaration->getASTContext()));

    for (const auto *param : declaration->parameters()) {
        auto parameter_type =
            common::to_string(param->getType(), param->getASTContext());
        common::ensure_lambda_type_is_relative(config(), parameter_type);
        parameter_type = simplify_system_template(parameter_type);
        method_model_ptr->add_parameter(config().using_namespace().relative(
            simplify_system_template(parameter_type)));
    }

    return method_model_ptr;
}

std::unique_ptr<clanguml::sequence_diagram::model::method>
translation_unit_visitor::create_method_model(clang::CXXMethodDecl *declaration)
{
    auto method_model_ptr = std::make_unique<sequence_diagram::model::method>(
        config().using_namespace());

    common::model::namespace_ ns{declaration->getQualifiedNameAsString()};
    auto method_name = ns.name();
    method_model_ptr->set_method_name(method_name);
    ns.pop_back();
    method_model_ptr->set_name(ns.name());
    ns.pop_back();

    method_model_ptr->is_defaulted(declaration->isDefaulted());
    method_model_ptr->is_assignment(declaration->isCopyAssignmentOperator() ||
        declaration->isMoveAssignmentOperator());
    method_model_ptr->is_const(declaration->isConst());
    method_model_ptr->is_static(declaration->isStatic());
    method_model_ptr->is_operator(declaration->isOverloadedOperator());
    method_model_ptr->is_constructor(
        clang::dyn_cast<clang::CXXConstructorDecl>(declaration) != nullptr);

    clang::Decl *parent_decl = declaration->getParent();

    if (context().current_class_template_decl_ != nullptr)
        parent_decl = context().current_class_template_decl_;

    LOG_DBG("Getting method's class with local id {}", parent_decl->getID());

    const auto maybe_method_class = get_participant<model::class_>(parent_decl);

    if (!maybe_method_class) {
        LOG_DBG("Cannot find parent class_ for method {} in class {}",
            declaration->getQualifiedNameAsString(),
            declaration->getParent()->getQualifiedNameAsString());
        return {};
    }

    const auto &method_class = maybe_method_class.value();

    method_model_ptr->is_void(declaration->getReturnType()->isVoidType());

    method_model_ptr->set_class_id(method_class.id());
    method_model_ptr->set_class_full_name(method_class.full_name(false));
    method_model_ptr->set_name(get_participant(method_model_ptr->class_id())
                                   .value()
                                   .full_name_no_ns() +
        "::" + declaration->getNameAsString());

    method_model_ptr->return_type(common::to_string(
        declaration->getReturnType(), declaration->getASTContext()));

    for (const auto *param : declaration->parameters()) {
        auto parameter_type =
            common::to_string(param->getType(), param->getASTContext());
        common::ensure_lambda_type_is_relative(config(), parameter_type);
        parameter_type = simplify_system_template(parameter_type);
        method_model_ptr->add_parameter(config().using_namespace().relative(
            simplify_system_template(parameter_type)));
    }

    return method_model_ptr;
}

bool translation_unit_visitor::should_include(const clang::TagDecl *decl) const
{
    if (source_manager().isInSystemHeader(decl->getSourceRange().getBegin()))
        return false;

    const auto decl_file = decl->getLocation().printToString(source_manager());

    return diagram().should_include(
               namespace_{decl->getQualifiedNameAsString()}) &&
        diagram().should_include(common::model::source_file{decl_file});
}

bool translation_unit_visitor::should_include(
    const clang::LambdaExpr *expr) const
{
    const auto expr_file = expr->getBeginLoc().printToString(source_manager());
    return diagram().should_include(common::model::source_file{expr_file});
}

bool translation_unit_visitor::should_include(const clang::CallExpr *expr) const
{
    if (context().caller_id() == 0)
        return false;

    // Skip casts, moves and such
    if (expr->isCallToStdMove())
        return false;

    if (expr->isImplicitCXXThis())
        return false;

    if (clang::dyn_cast_or_null<clang::ImplicitCastExpr>(expr) != nullptr)
        return false;

    if (!context().valid())
        return false;

    const auto expr_file = expr->getBeginLoc().printToString(source_manager());

    if (!diagram().should_include(common::model::source_file{expr_file}))
        return false;

    const auto *callee_decl = expr->getCalleeDecl();

    if (callee_decl != nullptr) {
        const auto *callee_function = callee_decl->getAsFunction();

        if ((callee_function == nullptr) || !should_include(callee_function))
            return false;

        return should_include(callee_function);
    }

    return true;
}

bool translation_unit_visitor::should_include(
    const clang::CXXMethodDecl *decl) const
{
    if (!should_include(decl->getParent()))
        return false;

    if (!diagram().should_include(
            common::access_specifier_to_access_t(decl->getAccess())))
        return false;

    LOG_DBG("Including method {}", decl->getQualifiedNameAsString());

    return true;
}

bool translation_unit_visitor::should_include(
    const clang::FunctionDecl *decl) const
{
    const auto decl_file = decl->getLocation().printToString(source_manager());

    return diagram().should_include(
               namespace_{decl->getQualifiedNameAsString()}) &&
        diagram().should_include(common::model::source_file{decl_file});
}

bool translation_unit_visitor::should_include(
    const clang::FunctionTemplateDecl *decl) const
{
    return should_include(decl->getAsFunction());
}

bool translation_unit_visitor::should_include(
    const clang::ClassTemplateDecl *decl) const
{
    if (source_manager().isInSystemHeader(decl->getSourceRange().getBegin()))
        return false;

    const auto decl_file = decl->getLocation().printToString(source_manager());

    return diagram().should_include(
               namespace_{decl->getQualifiedNameAsString()}) &&
        diagram().should_include(common::model::source_file{decl_file});
}

std::optional<std::string> translation_unit_visitor::get_expression_comment(
    const clang::SourceManager &sm, const clang::ASTContext &context,
    const eid_t caller_id, const clang::Stmt *stmt)
{
    const auto *raw_comment =
        clanguml::common::get_expression_raw_comment(sm, context, stmt);

    if (raw_comment == nullptr)
        return {};

    if (!caller_id.is_global() &&
        !processed_comments_by_caller_id_
             .emplace(caller_id.ast_local_value(), raw_comment)
             .second) {
        return {};
    }

    const auto &[decorators, stripped_comment] = decorators::parse(
        raw_comment->getFormattedText(sm, sm.getDiagnostics()));

    if (stripped_comment.empty())
        return {};

    return stripped_comment;
}
} // namespace clanguml::sequence_diagram::visitor
