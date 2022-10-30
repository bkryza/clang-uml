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
    : common::visitor::translation_unit_visitor{sm, config}
    , diagram_{diagram}
    , config_{config}
    , call_expression_context_{}
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
    // Skip system headers
    if (source_manager().isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    if (!diagram().should_include(cls->getQualifiedNameAsString()))
        return true;

    call_expression_context_.reset();

    if (!diagram().should_include(cls->getQualifiedNameAsString())) {
        return true;
    }

    if (cls->isTemplated() && cls->getDescribedTemplate()) {
        // If the described templated of this class is already in the model
        // skip it:
        if (get_ast_local_id(cls->getDescribedTemplate()->getID()))
            return true;
    }

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass())
        return true;

    // Build the class declaration and store it in the diagram, even
    // if we don't need it for any of the participants of this diagram
    auto c_ptr = create_class_declaration(cls);

    if (!c_ptr)
        return true;

    const auto cls_id = c_ptr->id();

    set_ast_local_id(cls->getID(), cls_id);

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
    else {
        forward_declarations_.erase(id);
    }

    if (diagram_.should_include(class_model)) {
        LOG_DBG("Adding class {} with id {}", class_model.full_name(false),
            class_model.id());

        assert(class_model.id() == cls_id);

        call_expression_context_.set_caller_id(cls_id);
        call_expression_context_.update(cls);

        diagram_.add_participant(std::move(c_ptr));
    }
    else {
        LOG_DBG("Skipping class {} with id {}", class_model.full_name(),
            class_model.id());
    }

    //    call_expression_context_.current_class_decl_ = cls;
    //    call_expression_context_.current_class_ = process_class(cls);

    return true;
}

bool translation_unit_visitor::VisitClassTemplateDecl(
    clang::ClassTemplateDecl *cls)
{
    if (source_manager().isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    if (!diagram().should_include(cls->getQualifiedNameAsString()))
        return true;

    LOG_DBG("= Visiting class template declaration {} at {}",
        cls->getQualifiedNameAsString(),
        cls->getLocation().printToString(source_manager()));

    auto c_ptr = create_class_declaration(cls->getTemplatedDecl());

    if (!c_ptr)
        return true;

    // Override the id with the template id, for now we don't care about the
    // underlying templated class id

    process_template_parameters(*cls, *c_ptr);

    const auto cls_full_name = c_ptr->full_name(false);
    const auto id = common::to_id(cls_full_name);

    c_ptr->set_id(id);

    set_ast_local_id(cls->getID(), id);

    if (!cls->getTemplatedDecl()->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(c_ptr));
        return true;
    }
    else {
        forward_declarations_.erase(id);
    }

    if (diagram_.should_include(*c_ptr)) {
        const auto name = c_ptr->full_name();
        LOG_DBG("Adding class template {} with id {}", name, id);

        call_expression_context_.set_caller_id(id);
        call_expression_context_.update(cls);

        diagram_.add_participant(std::move(c_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitCXXMethodDecl(clang::CXXMethodDecl *m)
{
    if (call_expression_context_.current_class_decl_ == nullptr ||
        call_expression_context_.current_class_template_decl_ == nullptr)
        return true;

    call_expression_context_.update(m);

    auto m_ptr = std::make_unique<sequence_diagram::model::method>(
        config().using_namespace());

    common::model::namespace_ ns{m->getQualifiedNameAsString()};
    m_ptr->set_method_name(ns.name());
    ns.pop_back();
    m_ptr->set_name(ns.name());
    ns.pop_back();
    m_ptr->set_namespace(ns);
    m_ptr->set_id(common::to_id(m->getQualifiedNameAsString()));

    m_ptr->set_class_id(
        get_ast_local_id(call_expression_context_.current_class_decl_->getID())
            .value());

    call_expression_context_.update(m);

    call_expression_context_.set_caller_id(m_ptr->id());

    set_ast_local_id(m->getID(), m_ptr->id());

    diagram().add_participant(std::move(m_ptr));

    return true;
}

bool translation_unit_visitor::VisitFunctionDecl(clang::FunctionDecl *f)
{
    if (f->isCXXClassMember())
        return true;

    const auto function_name = f->getQualifiedNameAsString();

    if (!diagram().should_include(function_name))
        return true;

    LOG_DBG("Visiting function declaration {}", function_name);

    if (f->isTemplated() && f->getDescribedTemplate()) {
        // If the described templated of this function is already in the model
        // skip it:
        if (get_ast_local_id(f->getDescribedTemplate()->getID()))
            return true;
    }

    auto f_ptr = std::make_unique<sequence_diagram::model::function>(
        config().using_namespace());

    common::model::namespace_ ns{function_name};
    f_ptr->set_name(ns.name());
    ns.pop_back();
    f_ptr->set_namespace(ns);
    f_ptr->set_id(common::to_id(function_name));

    call_expression_context_.update(f);

    call_expression_context_.set_caller_id(f_ptr->id());

    set_ast_local_id(f->getID(), f_ptr->id());

    // TODO: Handle overloaded functions with different arguments
    diagram().add_participant(std::move(f_ptr));

    return true;
}

bool translation_unit_visitor::VisitFunctionTemplateDecl(
    clang::FunctionTemplateDecl *function_template)
{
    const auto function_name = function_template->getQualifiedNameAsString();

    if (!diagram().should_include(function_name))
        return true;

    LOG_DBG("Visiting function template declaration {} at {}", function_name,
        function_template->getLocation().printToString(source_manager()));

    auto f_ptr = std::make_unique<sequence_diagram::model::function_template>(
        config().using_namespace());

    common::model::namespace_ ns{function_name};
    f_ptr->set_name(ns.name());
    ns.pop_back();
    f_ptr->set_namespace(ns);

    process_template_parameters(*function_template, *f_ptr);

    f_ptr->set_id(common::to_id(f_ptr->full_name(false)));

    call_expression_context_.update(function_template);
    call_expression_context_.set_caller_id(f_ptr->id());

    set_ast_local_id(function_template->getID(), f_ptr->id());

    // TODO: Handle overloaded functions with different arguments
    diagram().add_participant(std::move(f_ptr));

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

    if (!call_expression_context_.valid())
        return true;

    message m;
    m.type = message_t::kCall;
    m.from = call_expression_context_.caller_id();
    m.from_name = diagram().participants.at(m.from)->full_name(false);

    const auto &current_ast_context =
        *call_expression_context_.get_ast_context();

    if (const auto *operator_call_expr =
            clang::dyn_cast_or_null<clang::CXXOperatorCallExpr>(expr);
        operator_call_expr != nullptr) {
        // TODO: Handle C++ operator calls
    }
    //
    // class method
    //
    else if (const auto *method_call_expr =
                 clang::dyn_cast_or_null<clang::CXXMemberCallExpr>(expr);
             method_call_expr != nullptr) {

        // Get callee declaration as methods parent
        const auto *method_decl = method_call_expr->getMethodDecl();
        std::string method_name = method_decl->getQualifiedNameAsString();

        const auto *callee_decl =
            method_decl ? method_decl->getParent() : nullptr;

        if (!(callee_decl &&
                diagram().should_include(
                    callee_decl->getQualifiedNameAsString())))
            return true;

        // TODO: The method can be called before it's declaration has been
        //       encountered by the visitor - for now it's not a problem
        //       as overloaded methods are not supported
        m.to = common::to_id(method_decl->getQualifiedNameAsString());
        m.to_name = callee_decl->getQualifiedNameAsString();
        m.message_name = method_decl->getNameAsString();
        m.return_type = method_call_expr->getCallReturnType(current_ast_context)
                            .getAsString();

        if (get_ast_local_id(callee_decl->getID()))
            diagram().add_active_participant(
                get_ast_local_id(callee_decl->getID()).value());
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
                    unresolved_expr->dump();
                    for (const auto *decl : unresolved_expr->decls()) {
                        if (clang::dyn_cast_or_null<
                                clang::FunctionTemplateDecl>(decl)) {

                            auto *ftd = clang::dyn_cast_or_null<
                                clang::FunctionTemplateDecl>(decl);

                            m.to_name = to_string(ftd);
                            m.to = get_ast_local_id(ftd->getID()).value();
                            auto message_name =
                                diagram()
                                    .get_participant<model::function_template>(
                                        m.to)
                                    .value()
                                    .full_name(false)
                                    .substr();
                            m.message_name =
                                message_name.substr(0, message_name.size() - 2);

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

            m.to_name = callee_function->getQualifiedNameAsString();
            m.message_name = callee_function->getNameAsString();
            m.to = get_ast_local_id(callee_function->getID()).value();
        }
        m.return_type =
            function_call_expr->getCallReturnType(current_ast_context)
                .getAsString();
    }
    else {
        return true;
    }

    if (m.from > 0 && m.to > 0) {
        if (diagram().sequences.find(m.from) == diagram().sequences.end()) {
            activity a;
            a.usr = m.from;
            a.from = m.from_name;
            diagram().sequences.insert({m.from, std::move(a)});
        }

        diagram().add_active_participant(m.from);
        diagram().add_active_participant(m.to);

        LOG_DBG("Found call {} from {} [{}] to {} [{}] ", m.message_name,
            m.from_name, m.from, m.to_name, m.to);

        diagram().sequences[m.from].messages.emplace_back(std::move(m));

        assert(!diagram().sequences.empty());
    }

    return true;
}

std::unique_ptr<clanguml::sequence_diagram::model::class_>
translation_unit_visitor::create_class_declaration(clang::CXXRecordDecl *cls)
{
    assert(cls != nullptr);

    auto c_ptr{std::make_unique<clanguml::sequence_diagram::model::class_>(
        config_.using_namespace())};
    auto &c = *c_ptr;

    // TODO: refactor to method get_qualified_name()
    auto qualified_name =
        cls->getQualifiedNameAsString(); //  common::get_qualified_name(*cls);

    if (!diagram().should_include(qualified_name))
        return {};

    auto ns = common::get_tag_namespace(*cls);

    const auto *parent = cls->getParent();

    if (parent && parent->isRecord()) {
        // Here we have 2 options, either:
        //  - the parent is a regular C++ class/struct
        //  - the parent is a class template declaration/specialization
        std::optional<common::model::diagram_element::id_t> id_opt;
        int64_t local_id =
            static_cast<const clang::RecordDecl *>(parent)->getID();

        // First check if the parent has been added to the diagram as regular
        // class
        id_opt = get_ast_local_id(local_id);

        // If not, check if the parent template declaration is in the model
        if (!id_opt) {
            local_id = static_cast<const clang::RecordDecl *>(parent)
                           ->getDescribedTemplate()
                           ->getID();
            if (static_cast<const clang::RecordDecl *>(parent)
                    ->getDescribedTemplate())
                id_opt = get_ast_local_id(local_id);
        }

        assert(id_opt);

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

    LOG_DBG("Processing class {} template parameters...",
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

void translation_unit_visitor::set_ast_local_id(
    int64_t local_id, common::model::diagram_element::id_t global_id)
{
    LOG_DBG("== Setting local element mapping {} --> {}", local_id, global_id);

    local_ast_id_map_[local_id] = global_id;
}

std::optional<common::model::diagram_element::id_t>
translation_unit_visitor::get_ast_local_id(int64_t local_id) const
{
    if (local_ast_id_map_.find(local_id) == local_ast_id_map_.end())
        return {};

    return local_ast_id_map_.at(local_id);
}
}
