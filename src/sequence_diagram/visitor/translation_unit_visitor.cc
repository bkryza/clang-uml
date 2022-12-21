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
    : common::visitor::translation_unit_visitor{sm, config}
    , diagram_{diagram}
    , config_{config}
    , call_expression_context_{}
{
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

clanguml::sequence_diagram::model::diagram &translation_unit_visitor::diagram()
{
    return diagram_;
}

const clanguml::sequence_diagram::model::diagram &
translation_unit_visitor::diagram() const
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
    // Skip system headers
    if (source_manager().isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    if (!should_include(cls))
        return true;

    if (cls->isTemplated() && cls->getDescribedTemplate()) {
        // If the described templated of this class is already in the model
        // skip it:
        auto local_id = cls->getDescribedTemplate()->getID();
        if (get_unique_id(local_id))
            return true;
    }

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass())
        return true;

    LOG_TRACE("Visiting class declaration at {}",
        cls->getBeginLoc().printToString(source_manager()));

    // Build the class declaration and store it in the diagram, even
    // if we don't need it for any of the participants of this diagram
    auto c_ptr = this->create_class_declaration(cls);

    if (!c_ptr)
        return true;

    context().reset();

    const auto cls_id = c_ptr->id();

    set_unique_id(cls->getID(), cls_id);

    auto &class_model =
        diagram()
            .get_participant<sequence_diagram::model::class_>(cls_id)
            .has_value()
        ? *diagram()
               .get_participant<sequence_diagram::model::class_>(cls_id)
               .get()
        : *c_ptr;

    auto id = class_model.id();
    if (!cls->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(c_ptr));
        return true;
    }
    forward_declarations_.erase(id);

    if (diagram().should_include(class_model)) {
        LOG_DBG("Adding class {} with id {}", class_model.full_name(false),
            class_model.id());

        assert(class_model.id() == cls_id);

        context().set_caller_id(cls_id);
        context().update(cls);

        diagram().add_participant(std::move(c_ptr));
    }
    else {
        LOG_DBG("Skipping class {} with id {}", class_model.full_name(),
            class_model.id());
    }

    return true;
}

bool translation_unit_visitor::VisitClassTemplateDecl(
    clang::ClassTemplateDecl *cls)
{
    if (!should_include(cls))
        return true;

    LOG_TRACE("Visiting class template declaration {} at {} [{}]",
        cls->getQualifiedNameAsString(),
        cls->getLocation().printToString(source_manager()), (void *)cls);

    auto c_ptr = create_class_declaration(cls->getTemplatedDecl());

    if (!c_ptr)
        return true;

    // Override the id with the template id, for now we don't care about the
    // underlying templated class id
    process_template_parameters(*cls, *c_ptr);

    const auto cls_full_name = c_ptr->full_name(false);
    const auto id = common::to_id(cls_full_name);

    c_ptr->set_id(id);

    set_unique_id(cls->getID(), id);

    if (!cls->getTemplatedDecl()->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(c_ptr));
        return true;
    }
    forward_declarations_.erase(id);

    if (diagram().should_include(*c_ptr)) {
        LOG_DBG("Adding class template {} with id {}", cls_full_name, id);

        context().set_caller_id(id);
        context().update(cls);

        diagram().add_participant(std::move(c_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitClassTemplateSpecializationDecl(
    clang::ClassTemplateSpecializationDecl *cls)
{
    if (!should_include(cls))
        return true;

    LOG_TRACE("Visiting template specialization declaration {} at {}",
        cls->getQualifiedNameAsString(),
        cls->getLocation().printToString(source_manager()));

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass())
        return true;

    auto template_specialization_ptr = process_template_specialization(cls);

    if (!template_specialization_ptr)
        return true;

    const auto cls_full_name = template_specialization_ptr->full_name(false);
    const auto id = common::to_id(cls_full_name);

    template_specialization_ptr->set_id(id);

    set_unique_id(cls->getID(), id);

    if (!cls->isCompleteDefinition()) {
        forward_declarations_.emplace(
            id, std::move(template_specialization_ptr));
        return true;
    }
    forward_declarations_.erase(id);

    if (diagram().should_include(*template_specialization_ptr)) {
        LOG_DBG("Adding class template specialization {} with id {}",
            cls_full_name, id);

        context().set_caller_id(id);
        context().update(cls);

        diagram().add_participant(std::move(template_specialization_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitCXXMethodDecl(clang::CXXMethodDecl *m)
{
    if (!should_include(m))
        return true;

    if (!m->isThisDeclarationADefinition()) {
        if (m->getDefinition())
            return VisitCXXMethodDecl(
                static_cast<clang::CXXMethodDecl *>(m->getDefinition()));
    }

    LOG_TRACE("Visiting method {} in class {} [{}]",
        m->getQualifiedNameAsString(),
        m->getParent()->getQualifiedNameAsString(), (void *)m->getParent());

    context().update(m);

    auto m_ptr = std::make_unique<sequence_diagram::model::method>(
        config().using_namespace());

    common::model::namespace_ ns{m->getQualifiedNameAsString()};
    auto method_name = ns.name();
    m_ptr->set_method_name(method_name);
    ns.pop_back();
    m_ptr->set_name(ns.name());
    ns.pop_back();

    clang::Decl *parent_decl = m->getParent();

    if (context().current_class_template_decl_)
        parent_decl = context().current_class_template_decl_;

    LOG_DBG("Getting method's class with local id {}", parent_decl->getID());

    if (!get_participant<model::class_>(parent_decl)) {
        LOG_DBG("Cannot find parent class_ for method {} in class {}",
            m->getQualifiedNameAsString(),
            m->getParent()->getQualifiedNameAsString());
        return true;
    }

    const auto &method_class =
        get_participant<model::class_>(parent_decl).value();

    m_ptr->is_void(m->getReturnType()->isVoidType());

    m_ptr->set_class_id(method_class.id());
    m_ptr->set_class_full_name(method_class.full_name(false));
    m_ptr->set_name(
        get_participant(m_ptr->class_id()).value().full_name_no_ns() +
        "::" + m->getNameAsString());
    m_ptr->is_static(m->isStatic());

    for (const auto *param : m->parameters()) {
        m_ptr->add_parameter(config().using_namespace().relative(
            simplify_system_template(common::to_string(
                param->getType(), m->getASTContext(), false))));
    }

    process_comment(*m, *m_ptr);

    set_source_location(*m, *m_ptr);

    const auto method_full_name = m_ptr->full_name(false);

    m_ptr->set_id(common::to_id(method_full_name));

    // Callee methods in call expressions are referred to by first declaration
    // id
    if (m->isThisDeclarationADefinition()) {
        set_unique_id(m->getFirstDecl()->getID(), m_ptr->id());
    }

    set_unique_id(m->getID(), m_ptr->id()); // This is probably not necessary?

    LOG_TRACE("Set id {} --> {} for method name {} [{}]", m->getID(),
        m_ptr->id(), method_full_name, m->isThisDeclarationADefinition());

    context().update(m);

    context().set_caller_id(m_ptr->id());

    diagram().add_participant(std::move(m_ptr));

    return true;
}

bool translation_unit_visitor::VisitFunctionDecl(clang::FunctionDecl *f)
{
    if (f->isCXXClassMember())
        return true;

    if (!should_include(f))
        return true;

    const auto function_name = f->getQualifiedNameAsString();

    if (!f->isThisDeclarationADefinition()) {
        if (f->getDefinition())
            return VisitFunctionDecl(
                static_cast<clang::FunctionDecl *>(f->getDefinition()));
    }

    LOG_TRACE("Visiting function declaration {} at {}", function_name,
        f->getLocation().printToString(source_manager()));

    if (f->isTemplated()) {
        if (f->getDescribedTemplate()) {
            // If the described templated of this function is already in the
            // model skip it:
            if (get_unique_id(f->getDescribedTemplate()->getID()))
                return true;
        }
    }

    std::unique_ptr<model::function> f_ptr{};

    if (f->isFunctionTemplateSpecialization()) {
        f_ptr = build_function_template_instantiation(*f);
    }
    else {
        f_ptr = std::make_unique<sequence_diagram::model::function>(
            config().using_namespace());

        common::model::namespace_ ns{function_name};
        f_ptr->set_name(ns.name());
        ns.pop_back();
        f_ptr->set_namespace(ns);

        for (const auto *param : f->parameters()) {
            f_ptr->add_parameter(simplify_system_template(common::to_string(
                param->getType(), f->getASTContext(), false)));
        }
    }

    f_ptr->set_id(common::to_id(f_ptr->full_name(false)));

    f_ptr->is_void(f->getReturnType()->isVoidType());

    context().update(f);

    context().set_caller_id(f_ptr->id());

    if (f->isThisDeclarationADefinition()) {
        set_unique_id(f->getFirstDecl()->getID(), f_ptr->id());
    }
    set_unique_id(f->getID(), f_ptr->id());

    process_comment(*f, *f_ptr);

    set_source_location(*f, *f_ptr);

    diagram().add_participant(std::move(f_ptr));

    return true;
}

bool translation_unit_visitor::VisitFunctionTemplateDecl(
    clang::FunctionTemplateDecl *function_template)
{
    if (!should_include(function_template))
        return true;

    const auto function_name = function_template->getQualifiedNameAsString();

    LOG_TRACE("Visiting function template declaration {} at {}", function_name,
        function_template->getLocation().printToString(source_manager()));

    auto f_ptr = std::make_unique<sequence_diagram::model::function_template>(
        config().using_namespace());

    common::model::namespace_ ns{function_name};
    f_ptr->set_name(ns.name());
    ns.pop_back();
    f_ptr->set_namespace(ns);

    process_template_parameters(*function_template, *f_ptr);

    for (const auto *param :
        function_template->getTemplatedDecl()->parameters()) {
        f_ptr->add_parameter(simplify_system_template(common::to_string(
            param->getType(), function_template->getASTContext(), false)));
    }

    process_comment(*function_template, *f_ptr);

    set_source_location(*function_template, *f_ptr);

    f_ptr->set_id(common::to_id(f_ptr->full_name(false)));

    f_ptr->is_void(
        function_template->getAsFunction()->getReturnType()->isVoidType());

    context().update(function_template);

    context().set_caller_id(f_ptr->id());

    set_unique_id(function_template->getID(), f_ptr->id());

    diagram().add_participant(std::move(f_ptr));

    return true;
}

bool translation_unit_visitor::VisitLambdaExpr(clang::LambdaExpr *expr)
{
    if (!should_include(expr))
        return true;

    const auto lambda_full_name =
        expr->getLambdaClass()->getCanonicalDecl()->getNameAsString();

    LOG_TRACE("Visiting lambda expression {} at {}", lambda_full_name,
        expr->getBeginLoc().printToString(source_manager()));

    LOG_TRACE("Lambda call operator ID {} - lambda class ID {}, class call "
              "operator ID {}",
        expr->getCallOperator()->getID(), expr->getLambdaClass()->getID(),
        expr->getLambdaClass()->getLambdaCallOperator()->getID());

    // Create lambda class participant
    auto *cls = expr->getLambdaClass();
    auto c_ptr = create_class_declaration(cls);

    if (!c_ptr)
        return true;

    const auto cls_id = c_ptr->id();

    set_unique_id(cls->getID(), cls_id);

    // Create lambda class operator() participant
    auto m_ptr = std::make_unique<sequence_diagram::model::method>(
        config().using_namespace());

    auto method_name = "operator()";
    m_ptr->set_method_name(method_name);

    m_ptr->set_class_id(cls_id);
    m_ptr->set_class_full_name(c_ptr->full_name(false));

    diagram().add_participant(std::move(c_ptr));

    m_ptr->set_id(common::to_id(
        get_participant(cls_id).value().full_name(false) + "::" + method_name));

    context().enter_lambda_expression(m_ptr->id());

    set_unique_id(expr->getCallOperator()->getID(), m_ptr->id());

    diagram().add_participant(std::move(m_ptr));

    [[maybe_unused]] const auto is_generic_lambda = expr->isGenericLambda();

    return true;
}

bool translation_unit_visitor::TraverseLambdaExpr(clang::LambdaExpr *expr)
{
    const auto lambda_full_name =
        expr->getLambdaClass()->getCanonicalDecl()->getNameAsString();

    RecursiveASTVisitor<translation_unit_visitor>::TraverseLambdaExpr(expr);

    context().leave_lambda_expression();

    return true;
}

bool translation_unit_visitor::TraverseCallExpr(clang::CallExpr *expr)
{
    context().enter_callexpr(expr);

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCallExpr(expr);

    context().leave_callexpr();

    pop_message_to_diagram(expr);

    return true;
}

bool translation_unit_visitor::TraverseCXXMemberCallExpr(
    clang::CXXMemberCallExpr *expr)
{
    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXMemberCallExpr(
        expr);

    pop_message_to_diagram(expr);

    return true;
}

bool translation_unit_visitor::TraverseCXXOperatorCallExpr(
    clang::CXXOperatorCallExpr *expr)
{
    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXOperatorCallExpr(
        expr);

    pop_message_to_diagram(expr);

    return true;
}

bool translation_unit_visitor::TraverseCompoundStmt(clang::CompoundStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::common::model::namespace_;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto *current_ifstmt = context().current_ifstmt();
    const auto *current_elseifstmt = context().current_elseifstmt();

    //
    // Add final else block (not else if)
    //
    if (current_elseifstmt != nullptr) {
        if (current_elseifstmt->getElse() == stmt) {
            const auto current_caller_id = context().caller_id();

            if (current_caller_id) {
                diagram()
                    .get_activity(current_caller_id)
                    .add_message({message_t::kElse, current_caller_id});
            }
        }
    }
    else if (current_ifstmt != nullptr) {
        if (current_ifstmt->getElse() == stmt) {
            const auto current_caller_id = context().caller_id();

            if (current_caller_id) {
                diagram()
                    .get_activity(current_caller_id)
                    .add_message({message_t::kElse, current_caller_id});
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

    // Check if this is a beginning of a new if statement, or an
    // else if condition of the current if statement
    if (current_ifstmt != nullptr) {
        for (const auto *child_stmt : current_ifstmt->children()) {
            if (child_stmt == stmt) {
                elseif_block = true;
                break;
            }
        }
    }

    if (current_caller_id && !stmt->isConstexpr()) {
        context().enter_ifstmt(stmt);
        if (elseif_block) {
            context().enter_elseifstmt(stmt);
            diagram().add_if_stmt(current_caller_id, message_t::kElseIf);
        }
        else {
            diagram().add_if_stmt(current_caller_id, message_t::kIf);
        }
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseIfStmt(stmt);

    if (current_caller_id && !stmt->isConstexpr() && !elseif_block) {
        diagram().end_if_stmt(current_caller_id, message_t::kIfEnd);
    }

    return true;
}

bool translation_unit_visitor::TraverseWhileStmt(clang::WhileStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    if (current_caller_id) {
        context().enter_loopstmt(stmt);
        diagram().add_while_stmt(current_caller_id);
    }
    RecursiveASTVisitor<translation_unit_visitor>::TraverseWhileStmt(stmt);

    if (current_caller_id) {
        diagram().end_while_stmt(current_caller_id);
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

    if (current_caller_id) {
        context().enter_loopstmt(stmt);
        diagram().add_do_stmt(current_caller_id);
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseDoStmt(stmt);

    if (current_caller_id) {
        context().leave_loopstmt();
        diagram().end_do_stmt(current_caller_id);
    }

    return true;
}

bool translation_unit_visitor::TraverseForStmt(clang::ForStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    if (current_caller_id) {
        context().enter_loopstmt(stmt);
        diagram().add_for_stmt(current_caller_id);
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseForStmt(stmt);

    if (current_caller_id) {
        context().leave_loopstmt();
        diagram().end_for_stmt(current_caller_id);
    }

    return true;
}

bool translation_unit_visitor::TraverseCXXTryStmt(clang::CXXTryStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    if (current_caller_id) {
        context().enter_trystmt(stmt);
        diagram().add_try_stmt(current_caller_id);
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXTryStmt(stmt);

    if (current_caller_id) {
        context().leave_trystmt();
        diagram().end_try_stmt(current_caller_id);
    }

    return true;
}

bool translation_unit_visitor::TraverseCXXCatchStmt(clang::CXXCatchStmt *stmt)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::message;

    const auto current_caller_id = context().caller_id();

    if (current_caller_id && context().current_trystmt()) {
        std::string caught_type;
        if (stmt->getCaughtType().isNull())
            caught_type = "...";
        else
            caught_type = common::to_string(
                stmt->getCaughtType(), *context().get_ast_context());

        diagram().add_catch_stmt(current_caller_id, std::move(caught_type));
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

    if (current_caller_id) {
        context().enter_loopstmt(stmt);
        diagram().add_for_stmt(current_caller_id);
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCXXForRangeStmt(
        stmt);

    if (current_caller_id) {
        context().leave_loopstmt();
        diagram().end_for_stmt(current_caller_id);
    }

    return true;
}

bool translation_unit_visitor::TraverseSwitchStmt(clang::SwitchStmt *stmt)
{
    const auto current_caller_id = context().caller_id();

    if (current_caller_id) {
        context().enter_switchstmt(stmt);
        diagram().add_switch_stmt(current_caller_id);
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseSwitchStmt(stmt);

    if (current_caller_id) {
        context().leave_switchstmt();
        diagram().end_switch_stmt(current_caller_id);
    }

    return true;
}

bool translation_unit_visitor::TraverseCaseStmt(clang::CaseStmt *stmt)
{
    const auto current_caller_id = context().caller_id();

    if (current_caller_id) {
        diagram().add_case_stmt(
            current_caller_id, common::to_string(stmt->getLHS()));
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseCaseStmt(stmt);

    return true;
}

bool translation_unit_visitor::TraverseDefaultStmt(clang::DefaultStmt *stmt)
{
    const auto current_caller_id = context().caller_id();

    if (current_caller_id) {
        diagram().add_default_stmt(current_caller_id);
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseDefaultStmt(stmt);

    return true;
}

bool translation_unit_visitor::TraverseConditionalOperator(
    clang::ConditionalOperator *stmt)
{
    const auto current_caller_id = context().caller_id();

    if (current_caller_id) {
        context().enter_conditionaloperator(stmt);
        diagram().add_conditional_stmt(current_caller_id);
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseStmt(
        stmt->getCond());

    RecursiveASTVisitor<translation_unit_visitor>::TraverseStmt(
        stmt->getTrueExpr());

    if (current_caller_id) {
        diagram().add_conditional_elsestmt(current_caller_id);
    }

    RecursiveASTVisitor<translation_unit_visitor>::TraverseStmt(
        stmt->getFalseExpr());

    if (current_caller_id) {
        context().leave_conditionaloperator();
        diagram().end_conditional_stmt(current_caller_id);
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

    if (!should_include(expr))
        return true;

    LOG_TRACE("Visiting call expression at {} [caller_id = {}]",
        expr->getBeginLoc().printToString(source_manager()),
        context().caller_id());

    message m{message_t::kCall, context().caller_id()};

    set_source_location(*expr, m);

    // If we're currently inside a lambda expression, set it's id as
    // message source rather then enclosing context
    // Unless the lambda is declared in a function or method call
    if (context().lambda_caller_id() != 0) {
        if (context().current_callexpr() == nullptr) {
            m.set_from(context().lambda_caller_id());
        }
        else {
            LOG_DBG("Current lambda declaration is passed to a method or "
                    "function - keep the original caller id");
        }
    }

    if (context().is_expr_in_current_control_statement_condition(expr)) {
        m.set_message_scope(common::model::message_scope_t::kCondition);
    }

    //
    // Call to an overloaded operator
    //
    if (const auto *operator_call_expr =
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
            LOG_DBG("Cannot get callee declaration - trying direct callee...");
            callee_decl = expr->getDirectCallee();
        }

        if (!callee_decl) {
            //
            // Call to a method of a class template
            //
            if (clang::dyn_cast_or_null<clang::CXXDependentScopeMemberExpr>(
                    expr->getCallee())) {
                if (!process_class_template_method_call_expression(m, expr)) {
                    return true;
                }
            }
            //
            // Unresolved lookup expression are sometimes calls to template
            // functions
            //
            else if (clang::dyn_cast_or_null<clang::UnresolvedLookupExpr>(
                         expr->getCallee())) {
                if (!process_unresolved_lookup_call_expression(m, expr))
                    return true;
            }
        }
        else {
            if (!process_function_call_expression(m, expr)) {
                LOG_DBG("Skipping call to unsupported type of call expression "
                        "at: {}",
                    expr->getBeginLoc().printToString(source_manager()));

                return true;
            }
        }
    }

    //
    // This crashes on LLVM <= 12, for now just return empty type
    //
    // const auto &return_type =
    //    function_call_expr->getCallReturnType(current_ast_context);
    // m.return_type = return_type.getAsString();

    if (m.from() > 0 && m.to() > 0) {
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

bool translation_unit_visitor::process_operator_call_expression(
    model::message &m, const clang::CXXOperatorCallExpr *operator_call_expr)
{
    if (operator_call_expr->getCalleeDecl() == nullptr)
        return false;

    // For now we only handle call overloaded operators
    if (operator_call_expr->getOperator() !=
        clang::OverloadedOperatorKind::OO_Call)
        return false;

    LOG_DBG("Operator '{}' call expression to {} at {}",
        getOperatorSpelling(operator_call_expr->getOperator()),
        operator_call_expr->getCalleeDecl()->getID(),
        operator_call_expr->getBeginLoc().printToString(source_manager()));

    auto maybe_id = get_unique_id(operator_call_expr->getCalleeDecl()->getID());
    if (maybe_id.has_value()) {
        m.set_to(maybe_id.value());
    }
    else {
        m.set_to(operator_call_expr->getCalleeDecl()->getID());
    }

    m.set_message_name(fmt::format(
        "operator{}", getOperatorSpelling(operator_call_expr->getOperator())));

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

    auto *callee_decl = method_decl ? method_decl->getParent() : nullptr;

    if (!callee_decl)
        return false;

    if (!should_include(callee_decl) || !should_include(method_decl))
        return false;

    m.set_to(method_decl->getID());
    m.set_message_name(method_decl->getNameAsString());
    m.set_return_type(
        method_call_expr->getCallReturnType(*context().get_ast_context())
            .getAsString());

    LOG_TRACE("Set callee method id {} for method name {}", m.to(),
        method_decl->getQualifiedNameAsString());

    diagram().add_active_participant(method_decl->getID());

    return true;
}

bool translation_unit_visitor::process_class_template_method_call_expression(
    model::message &m, const clang::CallExpr *expr)
{
    auto *dependent_member_callee =
        clang::dyn_cast_or_null<clang::CXXDependentScopeMemberExpr>(
            expr->getCallee());

    if (is_callee_valid_template_specialization(dependent_member_callee)) {
        const auto *template_declaration =
            dependent_member_callee->getBaseType()
                ->getAs<clang::TemplateSpecializationType>()
                ->getTemplateName()
                .getAsTemplateDecl();

        std::string callee_method_full_name;

        // First check if the primary template is already in the
        // participants map
        if (get_participant(template_declaration).has_value()) {
            callee_method_full_name =
                get_participant(template_declaration).value().full_name(false) +
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
            else
                return false;
        }

        m.set_message_name(dependent_member_callee->getMember().getAsString());

        if (get_unique_id(template_declaration->getID()))
            diagram().add_active_participant(
                get_unique_id(template_declaration->getID()).value());
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

    if (!callee_function)
        return false;

    if (!should_include(callee_function))
        return false;

    // Skip free functions declared in files outside of included paths
    if (config().combine_free_functions_into_file_participants() &&
        !diagram().should_include(common::model::source_file{m.file()}))
        return false;

    auto callee_name = callee_function->getQualifiedNameAsString() + "()";

    std::unique_ptr<model::function_template> f_ptr;

    if (!get_unique_id(callee_function->getID()).has_value()) {
        // This is hopefully not an interesting call...
        m.set_to(callee_function->getID());
    }
    else {
        m.set_to(get_unique_id(callee_function->getID()).value());
    }

    auto message_name = callee_name;
    m.set_message_name(message_name.substr(0, message_name.size() - 2));

    if (f_ptr)
        diagram().add_participant(std::move(f_ptr));

    return true;
}

bool translation_unit_visitor::process_unresolved_lookup_call_expression(
    model::message &m, const clang::CallExpr *expr)
{
    // This is probably a template
    auto *unresolved_expr =
        clang::dyn_cast_or_null<clang::UnresolvedLookupExpr>(expr->getCallee());

    if (unresolved_expr) {
        for (const auto *decl : unresolved_expr->decls()) {
            if (clang::dyn_cast_or_null<clang::FunctionTemplateDecl>(decl)) {
                // Yes, it's a template
                auto *ftd =
                    clang::dyn_cast_or_null<clang::FunctionTemplateDecl>(decl);

                if (!get_unique_id(ftd->getID()).has_value())
                    m.set_to(ftd->getID());
                else {
                    m.set_to(get_unique_id(ftd->getID()).value());
                }

                break;
            }
        }
    }

    return true;
}

bool translation_unit_visitor::is_callee_valid_template_specialization(
    const clang::CXXDependentScopeMemberExpr *dependent_member_expr) const
{
    const bool base_type_is_not_null =
        !dependent_member_expr->getBaseType().isNull();

    const bool base_type_is_specialization_type =
        dependent_member_expr->getBaseType()
            ->getAs<clang::TemplateSpecializationType>() != nullptr;

    const bool base_type_is_not_pointer_type =
        base_type_is_specialization_type &&
        !dependent_member_expr->getBaseType()
             ->getAs<clang::TemplateSpecializationType>()
             ->isPointerType();

    return (base_type_is_not_null && base_type_is_specialization_type &&
        base_type_is_not_pointer_type);
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
translation_unit_visitor::create_class_declaration(clang::CXXRecordDecl *cls)
{
    assert(cls != nullptr);

    auto c_ptr{std::make_unique<clanguml::sequence_diagram::model::class_>(
        config().using_namespace())};
    auto &c = *c_ptr;

    // TODO: refactor to method get_qualified_name()
    auto qualified_name =
        cls->getQualifiedNameAsString(); //  common::get_qualified_name(*cls);

    if (!cls->isLambda())
        if (!diagram().should_include(qualified_name))
            return {};

    auto ns = common::get_tag_namespace(*cls);

    if (cls->isLambda() &&
        !diagram().should_include(ns.to_string() + "::lambda"))
        return {};

    const auto *parent = cls->getParent();

    if (parent && parent->isRecord()) {
        // Here we have 2 options, either:
        //  - the parent is a regular C++ class/struct
        //  - the parent is a class template declaration/specialization
        std::optional<common::model::diagram_element::id_t> id_opt;
        int64_t local_id =
            static_cast<const clang::RecordDecl *>(parent)->getID();

        // First check if the parent has been added to the diagram as
        // regular class
        id_opt = get_unique_id(local_id);

        // If not, check if the parent template declaration is in the model
        if (!id_opt &&
            static_cast<const clang::RecordDecl *>(parent)
                ->getDescribedTemplate()) {
            local_id = static_cast<const clang::RecordDecl *>(parent)
                           ->getDescribedTemplate()
                           ->getID();
            if (static_cast<const clang::RecordDecl *>(parent)
                    ->getDescribedTemplate())
                id_opt = get_unique_id(local_id);
        }

        if (!id_opt)
            return {};

        auto parent_class =
            diagram_.get_participant<clanguml::sequence_diagram::model::class_>(
                *id_opt);

        assert(parent_class);

        c.set_namespace(ns);
        if (cls->getNameAsString().empty()) {
            // Nested structs can be anonymous
            if (anonymous_struct_relationships_.count(cls->getID()) > 0) {
                const auto &[label, hint, access] =
                    anonymous_struct_relationships_[cls->getID()];

                c.set_name(parent_class.value().name() + "##" +
                    fmt::format("({})", label));

                parent_class.value().add_relationship(
                    {hint, common::to_id(c.full_name(false)), access, label});
            }
            else
                c.set_name(parent_class.value().name() + "##" +
                    fmt::format(
                        "(anonymous_{})", std::to_string(cls->getID())));
        }
        else {
            c.set_name(
                parent_class.value().name() + "##" + cls->getNameAsString());
        }

        c.set_id(common::to_id(c.full_name(false)));

        c.nested(true);
    }
    else if (cls->isLambda()) {
        c.is_lambda(true);
        if (cls->getParent()) {
            const auto type_name = make_lambda_name(cls);

            c.set_name(type_name);
            c.set_namespace(ns);
            c.set_id(common::to_id(c.full_name(false)));

            // Check if lambda is declared as an argument passed to a
            // function/method call
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

bool translation_unit_visitor::process_template_parameters(
    const clang::TemplateDecl &template_declaration,
    sequence_diagram::model::template_trait &c)
{
    using class_diagram::model::template_parameter;

    LOG_TRACE("Processing class {} template parameters...",
        common::get_qualified_name(template_declaration));

    if (template_declaration.getTemplateParameters() == nullptr)
        return false;

    for (const auto *parameter :
        *template_declaration.getTemplateParameters()) {
        if (clang::dyn_cast_or_null<clang::TemplateTypeParmDecl>(parameter)) {
            const auto *template_type_parameter =
                clang::dyn_cast_or_null<clang::TemplateTypeParmDecl>(parameter);
            template_parameter ct;
            ct.set_type("");
            ct.is_template_parameter(true);
            ct.set_name(template_type_parameter->getNameAsString());
            ct.set_default_value("");
            ct.is_variadic(template_type_parameter->isParameterPack());

            c.add_template(std::move(ct));
        }
        else if (clang::dyn_cast_or_null<clang::NonTypeTemplateParmDecl>(
                     parameter)) {
            const auto *template_nontype_parameter =
                clang::dyn_cast_or_null<clang::NonTypeTemplateParmDecl>(
                    parameter);
            template_parameter ct;
            ct.set_type(template_nontype_parameter->getType().getAsString());
            ct.set_name(template_nontype_parameter->getNameAsString());
            ct.is_template_parameter(false);
            ct.set_default_value("");
            ct.is_variadic(template_nontype_parameter->isParameterPack());

            c.add_template(std::move(ct));
        }
        else if (clang::dyn_cast_or_null<clang::TemplateTemplateParmDecl>(
                     parameter)) {
            const auto *template_template_parameter =
                clang::dyn_cast_or_null<clang::TemplateTemplateParmDecl>(
                    parameter);
            template_parameter ct;
            ct.set_type("");
            ct.set_name(template_template_parameter->getNameAsString() + "<>");
            ct.is_template_parameter(true);
            ct.set_default_value("");
            ct.is_variadic(template_template_parameter->isParameterPack());

            c.add_template(std::move(ct));
        }
        else {
            // pass
        }
    }

    return false;
}

void translation_unit_visitor::set_unique_id(
    int64_t local_id, common::model::diagram_element::id_t global_id)
{
    LOG_TRACE("Setting local element mapping {} --> {}", local_id, global_id);

    local_ast_id_map_[local_id] = global_id;
}

std::optional<common::model::diagram_element::id_t>
translation_unit_visitor::get_unique_id(int64_t local_id) const
{
    if (local_ast_id_map_.find(local_id) == local_ast_id_map_.end())
        return {};

    return local_ast_id_map_.at(local_id);
}

std::unique_ptr<model::function_template>
translation_unit_visitor::build_function_template_instantiation(
    const clang::FunctionDecl &decl)
{
    //
    // Here we'll hold the template base params to replace with the
    // instantiated values
    //
    std::deque<std::tuple</*parameter name*/ std::string, /* position */ int,
        /*is variadic */ bool>>
        template_base_params{};

    auto template_instantiation_ptr =
        std::make_unique<model::function_template>(config_.using_namespace());
    auto &template_instantiation = *template_instantiation_ptr;

    //
    // Set function template instantiation name
    //
    auto template_decl_qualified_name = decl.getQualifiedNameAsString();
    common::model::namespace_ ns{template_decl_qualified_name};
    ns.pop_back();
    template_instantiation.set_name(decl.getNameAsString());
    template_instantiation.set_namespace(ns);

    //
    // Instantiate the template arguments
    //
    model::template_trait *parent{nullptr};
    build_template_instantiation_process_template_arguments(parent,
        template_base_params, decl.getTemplateSpecializationArgs()->asArray(),
        template_instantiation, "", decl.getPrimaryTemplate());

    for (const auto *param : decl.parameters()) {
        template_instantiation_ptr->add_parameter(
            common::to_string(param->getType(), decl.getASTContext()));
    }

    return template_instantiation_ptr;
}

void translation_unit_visitor::
    build_template_instantiation_process_template_arguments(
        model::template_trait *parent,
        std::deque<std::tuple<std::string, int, bool>>
            & /*template_base_params*/,
        const clang::ArrayRef<clang::TemplateArgument> &template_args,
        model::template_trait &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl)
{
    auto arg_index = 0U;
    for (const auto &arg : template_args) {
        const auto argument_kind = arg.getKind();
        class_diagram::model::template_parameter argument;
        if (argument_kind == clang::TemplateArgument::Template) {
            build_template_instantiation_process_template_argument(
                arg, argument);
        }
        else if (argument_kind == clang::TemplateArgument::Type) {
            build_template_instantiation_process_type_argument(parent,
                full_template_specialization_name, template_decl, arg,
                template_instantiation, argument);
        }
        else if (argument_kind == clang::TemplateArgument::Integral) {
            build_template_instantiation_process_integral_argument(
                arg, argument);
        }
        else if (argument_kind == clang::TemplateArgument::Expression) {
            build_template_instantiation_process_expression_argument(
                arg, argument);
        }
        else {
            LOG_INFO("Unsupported argument type {}", arg.getKind());
        }

        simplify_system_template(
            argument, argument.to_string(config().using_namespace(), false));

        template_instantiation.add_template(std::move(argument));

        arg_index++;
    }
}

void translation_unit_visitor::
    build_template_instantiation_process_template_argument(
        const clang::TemplateArgument &arg,
        class_diagram::model::template_parameter &argument) const
{
    argument.is_template_parameter(true);
    auto arg_name =
        arg.getAsTemplate().getAsTemplateDecl()->getQualifiedNameAsString();
    argument.set_type(arg_name);
}

void translation_unit_visitor::
    build_template_instantiation_process_integral_argument(
        const clang::TemplateArgument &arg,
        class_diagram::model::template_parameter &argument) const
{
    assert(arg.getKind() == clang::TemplateArgument::Integral);

    argument.is_template_parameter(false);
    argument.set_type(std::to_string(arg.getAsIntegral().getExtValue()));
}

void translation_unit_visitor::
    build_template_instantiation_process_expression_argument(
        const clang::TemplateArgument &arg,
        class_diagram::model::template_parameter &argument) const
{
    assert(arg.getKind() == clang::TemplateArgument::Expression);

    argument.is_template_parameter(false);
    argument.set_type(common::get_source_text(
        arg.getAsExpr()->getSourceRange(), source_manager()));
}

void translation_unit_visitor::
    build_template_instantiation_process_tag_argument(
        model::template_trait & /*template_instantiation*/,
        const std::string & /*full_template_specialization_name*/,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        class_diagram::model::template_parameter &argument) const
{
    assert(arg.getKind() == clang::TemplateArgument::Type);

    argument.is_template_parameter(false);

    argument.set_name(
        common::to_string(arg.getAsType(), template_decl->getASTContext()));
}

void translation_unit_visitor::
    build_template_instantiation_process_type_argument(
        model::template_trait * /*parent*/,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg,
        model::template_trait &template_instantiation,
        class_diagram::model::template_parameter &argument)
{
    assert(arg.getKind() == clang::TemplateArgument::Type);

    argument.is_template_parameter(false);

    // If this is a nested template type - add nested templates as
    // template arguments
    if (arg.getAsType()->getAs<clang::FunctionType>()) {
        // TODO
    }
    else if (arg.getAsType()->getAs<clang::TemplateSpecializationType>()) {
        const auto *nested_template_type =
            arg.getAsType()->getAs<clang::TemplateSpecializationType>();

        const auto nested_template_name =
            nested_template_type->getTemplateName()
                .getAsTemplateDecl()
                ->getQualifiedNameAsString();

        auto [tinst_ns, tinst_name] = cx::util::split_ns(nested_template_name);

        argument.set_name(nested_template_name);

        // Check if this template should be simplified (e.g. system
        // template aliases such as 'std:basic_string<char>' should
        // be simply 'std::string')
        simplify_system_template(
            argument, argument.to_string(config().using_namespace(), false));
    }
    else if (arg.getAsType()->getAs<clang::TemplateTypeParmType>()) {
        argument.is_template_parameter(true);
        argument.set_name(
            common::to_string(arg.getAsType(), template_decl->getASTContext()));
    }
    else {
        // This is just a regular record type
        build_template_instantiation_process_tag_argument(
            template_instantiation, full_template_specialization_name,
            template_decl, arg, argument);
    }
}

std::unique_ptr<model::class_>
translation_unit_visitor::process_template_specialization(
    clang::ClassTemplateSpecializationDecl *cls)
{
    auto c_ptr{std::make_unique<model::class_>(config_.using_namespace())};
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

    if (template_instantiation.skip())
        return {};

    const auto template_args_count = cls->getTemplateArgs().size();
    for (auto arg_it = 0U; arg_it < template_args_count; arg_it++) {
        const auto arg = cls->getTemplateArgs().get(arg_it);
        process_template_specialization_argument(
            cls, template_instantiation, arg, arg_it);
    }

    template_instantiation.set_id(
        common::to_id(template_instantiation.full_name(false)));

    set_unique_id(cls->getID(), template_instantiation.id());

    return c_ptr;
}

void translation_unit_visitor::process_template_specialization_argument(
    const clang::ClassTemplateSpecializationDecl *cls,
    model::class_ &template_instantiation, const clang::TemplateArgument &arg,
    size_t argument_index, bool /*in_parameter_pack*/)
{
    const auto argument_kind = arg.getKind();

    if (argument_kind == clang::TemplateArgument::Type) {
        class_diagram::model::template_parameter argument;
        argument.is_template_parameter(false);

        // If this is a nested template type - add nested templates as
        // template arguments
        if (arg.getAsType()->getAs<clang::TemplateSpecializationType>()) {
            const auto *nested_template_type =
                arg.getAsType()->getAs<clang::TemplateSpecializationType>();

            const auto nested_template_name =
                nested_template_type->getTemplateName()
                    .getAsTemplateDecl()
                    ->getQualifiedNameAsString();

            argument.set_name(nested_template_name);

            auto nested_template_instantiation = build_template_instantiation(
                *arg.getAsType()->getAs<clang::TemplateSpecializationType>(),
                &template_instantiation);

            argument.set_id(nested_template_instantiation->id());

            for (const auto &t : nested_template_instantiation->templates())
                argument.add_template_param(t);

            // Check if this template should be simplified (e.g. system
            // template aliases such as 'std:basic_string<char>' should be
            // simply 'std::string')
            simplify_system_template(argument,
                argument.to_string(config().using_namespace(), false));
        }
        else if (arg.getAsType()->getAs<clang::TemplateTypeParmType>()) {
            auto type_name =
                common::to_string(arg.getAsType(), cls->getASTContext());

            // clang does not provide declared template parameter/argument
            // names in template specializations - so we have to extract
            // them from raw source code...
            if (type_name.find("type-parameter-") == 0) {
                auto declaration_text = common::get_source_text_raw(
                    cls->getSourceRange(), source_manager());

                declaration_text = declaration_text.substr(
                    declaration_text.find(cls->getNameAsString()) +
                    cls->getNameAsString().size() + 1);

                auto template_params =
                    cx::util::parse_unexposed_template_params(
                        declaration_text, [](const auto &t) { return t; });

                if (template_params.size() > argument_index)
                    type_name = template_params[argument_index].to_string(
                        config().using_namespace(), false);
                else {
                    LOG_DBG("Failed to find type specialization for argument "
                            "{} at index {} in declaration \n===\n{}\n===\n",
                        type_name, argument_index, declaration_text);
                }
            }

            argument.set_name(type_name);
        }
        else if (arg.getAsType()->getAsCXXRecordDecl() &&
            arg.getAsType()->getAsCXXRecordDecl()->isLambda()) {
            if (get_unique_id(arg.getAsType()->getAsCXXRecordDecl()->getID())
                    .has_value()) {
                argument.set_name(get_participant(
                    get_unique_id(
                        arg.getAsType()->getAsCXXRecordDecl()->getID())
                        .value())
                                      .value()
                                      .full_name(false));
            }
            else {
                const auto type_name =
                    make_lambda_name(arg.getAsType()->getAsCXXRecordDecl());
                argument.set_name(type_name);
            }
        }
        else {
            auto type_name =
                common::to_string(arg.getAsType(), cls->getASTContext());
            if (type_name.find('<') != std::string::npos) {
                // Sometimes template instantiation is reported as
                // RecordType in the AST and getAs to
                // TemplateSpecializationType returns null pointer so we
                // have to at least make sure it's properly formatted
                // (e.g. std:integral_constant, or any template
                // specialization which contains it - see t00038)
                process_unexposed_template_specialization_parameters(
                    type_name.substr(type_name.find('<') + 1,
                        type_name.size() - (type_name.find('<') + 2)),
                    argument, template_instantiation);

                argument.set_name(type_name.substr(0, type_name.find('<')));
            }
            else if (type_name.find("type-parameter-") == 0) {
                auto declaration_text = common::get_source_text_raw(
                    cls->getSourceRange(), source_manager());

                declaration_text = declaration_text.substr(
                    declaration_text.find(cls->getNameAsString()) +
                    cls->getNameAsString().size() + 1);

                auto template_params =
                    cx::util::parse_unexposed_template_params(
                        declaration_text, [](const auto &t) { return t; });

                if (template_params.size() > argument_index)
                    type_name = template_params[argument_index].to_string(
                        config().using_namespace(), false);
                else {
                    LOG_DBG("Failed to find type specialization for argument "
                            "{} at index {} in declaration \n===\n{}\n===\n",
                        type_name, argument_index, declaration_text);
                }

                // Otherwise just set the name for the template argument to
                // whatever clang says
                argument.set_name(type_name);
            }
            else
                argument.set_name(type_name);
        }

        LOG_TRACE("Adding template instantiation argument {}",
            argument.to_string(config().using_namespace(), false));

        simplify_system_template(
            argument, argument.to_string(config().using_namespace(), false));

        template_instantiation.add_template(std::move(argument));
    }
    else if (argument_kind == clang::TemplateArgument::Integral) {
        class_diagram::model::template_parameter argument;
        argument.is_template_parameter(false);
        argument.set_type(std::to_string(arg.getAsIntegral().getExtValue()));
        template_instantiation.add_template(std::move(argument));
    }
    else if (argument_kind == clang::TemplateArgument::Expression) {
        class_diagram::model::template_parameter argument;
        argument.is_template_parameter(false);
        argument.set_type(common::get_source_text(
            arg.getAsExpr()->getSourceRange(), source_manager()));
        template_instantiation.add_template(std::move(argument));
    }
    else if (argument_kind == clang::TemplateArgument::TemplateExpansion) {
        class_diagram::model::template_parameter argument;
        argument.is_template_parameter(true);

        cls->getLocation().dump(source_manager());
    }
    else if (argument_kind == clang::TemplateArgument::Pack) {
        // This will only work for now if pack is at the end
        size_t argument_pack_index{argument_index};
        for (const auto &template_argument : arg.getPackAsArray()) {
            process_template_specialization_argument(cls,
                template_instantiation, template_argument,
                argument_pack_index++, true);
        }
    }
    else {
        LOG_INFO("Unsupported template argument kind {} [{}]", arg.getKind(),
            cls->getLocation().printToString(source_manager()));
    }
}

std::unique_ptr<model::class_>
translation_unit_visitor::build_template_instantiation(
    const clang::TemplateSpecializationType &template_type_decl,
    model::class_ *parent)
{
    // TODO: Make sure we only build instantiation once

    //
    // Here we'll hold the template base params to replace with the
    // instantiated values
    //
    std::deque<std::tuple</*parameter name*/ std::string, /* position */ int,
        /*is variadic */ bool>>
        template_base_params{};

    auto *template_type_ptr = &template_type_decl;
    if (template_type_decl.isTypeAlias() &&
        template_type_decl.getAliasedType()
            ->getAs<clang::TemplateSpecializationType>())
        template_type_ptr = template_type_decl.getAliasedType()
                                ->getAs<clang::TemplateSpecializationType>();

    auto &template_type = *template_type_ptr;

    //
    // Create class_ instance to hold the template instantiation
    //
    auto template_instantiation_ptr =
        std::make_unique<model::class_>(config_.using_namespace());
    auto &template_instantiation = *template_instantiation_ptr;
    std::string full_template_specialization_name = common::to_string(
        template_type.desugar(),
        template_type.getTemplateName().getAsTemplateDecl()->getASTContext());

    auto *template_decl{template_type.getTemplateName().getAsTemplateDecl()};

    auto template_decl_qualified_name =
        template_decl->getQualifiedNameAsString();

    auto *class_template_decl{
        clang::dyn_cast<clang::ClassTemplateDecl>(template_decl)};

    if (class_template_decl && class_template_decl->getTemplatedDecl() &&
        class_template_decl->getTemplatedDecl()->getParent() &&
        class_template_decl->getTemplatedDecl()->getParent()->isRecord()) {

        common::model::namespace_ ns{
            common::get_tag_namespace(*class_template_decl->getTemplatedDecl()
                                           ->getParent()
                                           ->getOuterLexicalRecordContext())};

        std::string ns_str = ns.to_string();
        std::string name = template_decl->getQualifiedNameAsString();
        if (!ns_str.empty()) {
            name = name.substr(ns_str.size() + 2);
        }

        util::replace_all(name, "::", "##");
        template_instantiation.set_name(name);

        template_instantiation.set_namespace(ns);
    }
    else {
        common::model::namespace_ ns{template_decl_qualified_name};
        ns.pop_back();
        template_instantiation.set_name(template_decl->getNameAsString());
        template_instantiation.set_namespace(ns);
    }

    // TODO: Refactor handling of base parameters to a separate method

    // We need this to match any possible base classes coming from template
    // arguments
    std::vector<
        std::pair</* parameter name */ std::string, /* is variadic */ bool>>
        template_parameter_names{};

    for (const auto *parameter : *template_decl->getTemplateParameters()) {
        if (parameter->isTemplateParameter() &&
            (parameter->isTemplateParameterPack() ||
                parameter->isParameterPack())) {
            template_parameter_names.emplace_back(
                parameter->getNameAsString(), true);
        }
        else
            template_parameter_names.emplace_back(
                parameter->getNameAsString(), false);
    }

    // Check if the primary template has any base classes
    int base_index = 0;

    const auto *templated_class_decl =
        clang::dyn_cast_or_null<clang::CXXRecordDecl>(
            template_decl->getTemplatedDecl());

    if (templated_class_decl && templated_class_decl->hasDefinition())
        for (const auto &base : templated_class_decl->bases()) {
            const auto base_class_name = common::to_string(
                base.getType(), templated_class_decl->getASTContext(), false);

            LOG_DBG("Found template instantiation base: {}, {}",
                base_class_name, base_index);

            // Check if any of the primary template arguments has a
            // parameter equal to this type
            auto it = std::find_if(template_parameter_names.begin(),
                template_parameter_names.end(),
                [&base_class_name](
                    const auto &p) { return p.first == base_class_name; });

            if (it != template_parameter_names.end()) {
                const auto &parameter_name = it->first;
                const bool is_variadic = it->second;
                // Found base class which is a template parameter
                LOG_DBG("Found base class which is a template parameter "
                        "{}, {}, {}",
                    parameter_name, is_variadic,
                    std::distance(template_parameter_names.begin(), it));

                template_base_params.emplace_back(parameter_name,
                    std::distance(template_parameter_names.begin(), it),
                    is_variadic);
            }
            else {
                // This is a regular base class - it is handled by
                // process_template
            }
            base_index++;
        }

    build_template_instantiation_process_template_arguments(parent,
        template_base_params, template_type.template_arguments(),
        template_instantiation, full_template_specialization_name,
        template_decl);

    // First try to find the best match for this template in partially
    // specialized templates
    std::string destination{};
    std::string best_match_full_name{};
    auto full_template_name = template_instantiation.full_name(false);
    int best_match{};
    common::model::diagram_element::id_t best_match_id{0};

    for (const auto &[id, c] : diagram().participants()) {
        const auto *participant_as_class =
            dynamic_cast<model::class_ *>(c.get());
        if ((participant_as_class != nullptr) &&
            (*participant_as_class == template_instantiation))
            continue;

        auto c_full_name = participant_as_class->full_name(false);
        auto match =
            participant_as_class->calculate_template_specialization_match(
                template_instantiation, template_instantiation.name_and_ns());

        if (match > best_match) {
            best_match = match;
            best_match_full_name = c_full_name;
            best_match_id = participant_as_class->id();
        }
    }

    auto templated_decl_id =
        template_type.getTemplateName().getAsTemplateDecl()->getID();
    //    auto templated_decl_local_id =
    //        get_unique_id(templated_decl_id).value_or(0);

    if (best_match_id > 0) {
        destination = best_match_full_name;
    }
    else {
        LOG_DBG("Cannot determine global id for specialization template {} "
                "- delaying until the translation unit is complete ",
            templated_decl_id);
    }

    template_instantiation.set_id(
        common::to_id(template_instantiation_ptr->full_name(false)));

    return template_instantiation_ptr;
}

void translation_unit_visitor::
    process_unexposed_template_specialization_parameters(
        const std::string &type_name,
        class_diagram::model::template_parameter &tp,
        model::class_ & /*c*/) const
{
    auto template_params = cx::util::parse_unexposed_template_params(
        type_name, [](const std::string &t) { return t; });

    for (auto &param : template_params) {
        tp.add_template_param(param);
    }
}

bool translation_unit_visitor::simplify_system_template(
    class_diagram::model::template_parameter &ct, const std::string &full_name)
{
    if (config().type_aliases().count(full_name) > 0) {
        ct.set_name(config().type_aliases().at(full_name));
        ct.clear_params();
        return true;
    }
    return false;
}

std::string translation_unit_visitor::simplify_system_template(
    const std::string &full_name) const
{
    std::string result{full_name};
    for (const auto &[k, v] : config().type_aliases()) {
        util::replace_all(result, k, v);
    }

    return result;
}

std::string translation_unit_visitor::make_lambda_name(
    const clang::CXXRecordDecl *cls) const
{
    std::string result;
    const auto location = cls->getLocation();
    const auto file_line = source_manager().getSpellingLineNumber(location);
    const auto file_column = source_manager().getSpellingColumnNumber(location);
    const std::string file_name =
        util::split(source_manager().getFilename(location).str(), "/").back();

    if (context().caller_id() != 0 &&
        get_participant(context().caller_id()).has_value()) {
        auto parent_full_name =
            get_participant(context().caller_id()).value().full_name_no_ns();

        result = fmt::format("{}##(lambda {}:{}:{})", parent_full_name,
            file_name, file_line, file_column);
    }
    else {
        result =
            fmt::format("(lambda {}:{}:{})", file_name, file_line, file_column);
    }

    return result;
}

void translation_unit_visitor::push_message(
    clang::CallExpr *expr, model::message &&m)
{
    call_expr_message_map_.emplace(expr, std::move(m));
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
    diagram().get_activity(caller_id).add_message(std::move(msg));

    call_expr_message_map_.erase(expr);
}

void translation_unit_visitor::finalize()
{
    std::set<common::model::diagram_element::id_t> active_participants_unique;

    for (auto id : diagram().active_participants()) {
        if (local_ast_id_map_.find(id) != local_ast_id_map_.end()) {
            active_participants_unique.emplace(local_ast_id_map_.at(id));
        }
        else {
            active_participants_unique.emplace(id);
        }
    }

    diagram().active_participants() = std::move(active_participants_unique);

    for (auto &[id, activity] : diagram().sequences()) {
        for (auto &m : activity.messages()) {
            if (local_ast_id_map_.find(m.to()) != local_ast_id_map_.end()) {
                m.set_to(local_ast_id_map_.at(m.to()));
            }
        }
    }
}

bool translation_unit_visitor::should_include(const clang::TagDecl *decl) const
{
    if (source_manager().isInSystemHeader(decl->getSourceRange().getBegin()))
        return false;

    const auto decl_file = decl->getLocation().printToString(source_manager());

    return diagram().should_include(decl->getQualifiedNameAsString()) &&
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

    if (clang::dyn_cast_or_null<clang::ImplicitCastExpr>(expr))
        return false;

    if (!context().valid())
        return false;

    const auto expr_file = expr->getBeginLoc().printToString(source_manager());
    return diagram().should_include(common::model::source_file{expr_file});
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

    return diagram().should_include(decl->getQualifiedNameAsString()) &&
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

    return diagram().should_include(decl->getQualifiedNameAsString()) &&
        diagram().should_include(common::model::source_file{decl_file});
}
} // namespace clanguml::sequence_diagram::visitor
