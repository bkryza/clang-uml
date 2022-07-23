/**
 * src/class_diagram/visitor/translation_unit_visitor.cc
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
#include "cx/util.h"

#include <clang/Basic/FileManager.h>
#include <clang/Lex/Preprocessor.h>
#include <spdlog/spdlog.h>

namespace clanguml::class_diagram::visitor {

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_member;
using clanguml::class_diagram::model::class_method;
using clanguml::class_diagram::model::class_parent;
using clanguml::class_diagram::model::diagram;
using clanguml::class_diagram::model::enum_;
using clanguml::class_diagram::model::method_parameter;
using clanguml::class_diagram::model::template_parameter;
using clanguml::class_diagram::model::type_alias;
using clanguml::common::model::access_t;
using clanguml::common::model::decorated_element;
using clanguml::common::model::namespace_;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;

namespace detail {
access_t access_specifier_to_access_t(clang::AccessSpecifier access_specifier)
{
    auto access = access_t::kPublic;
    switch (access_specifier) {
    case clang::AccessSpecifier::AS_public:
        access = access_t::kPublic;
        break;
    case clang::AccessSpecifier::AS_private:
        access = access_t::kPrivate;
        break;
    case clang::AccessSpecifier::AS_protected:
        access = access_t::kProtected;
        break;
    default:
        break;
    }

    return access;
}

std::optional<clanguml::common::model::namespace_> get_enclosing_namespace(
    const clang::DeclContext *decl)
{
    if (!decl->getEnclosingNamespaceContext()->isNamespace())
        return {};

    const auto *namespace_declaration =
        clang::cast<clang::NamespaceDecl>(decl->getEnclosingNamespaceContext());

    if (namespace_declaration == nullptr) {
        return {};
    }

    return namespace_{namespace_declaration->getQualifiedNameAsString()};
}
}

std::string to_string(const clang::QualType &type, const clang::ASTContext &ctx)
{
    const clang::PrintingPolicy print_policy(ctx.getLangOpts());

    auto result{type.getAsString(print_policy)};

    if (result.find('<') != std::string::npos) {
        auto canonical_type_name =
            type.getCanonicalType().getAsString(print_policy);

        auto result_qualified_template_name =
            result.substr(0, result.find('<'));
        auto result_template_arguments = result.substr(result.find('<'));

        auto canonical_qualified_template_name =
            canonical_type_name.substr(0, canonical_type_name.find('<'));

        // Choose the longer name (why do I have to do this?)
        if (result_qualified_template_name.size() <
            canonical_qualified_template_name.size()) {

            result =
                canonical_qualified_template_name + result_template_arguments;
        }
    }

    return result;
}

std::string get_source_text_raw(
    clang::SourceRange range, const clang::SourceManager &sm)
{
    return clang::Lexer::getSourceText(
        clang::CharSourceRange::getCharRange(range), sm, clang::LangOptions())
        .str();
}

std::string get_source_text(
    clang::SourceRange range, const clang::SourceManager &sm)
{
    clang::LangOptions lo;

    // NOTE: sm.getSpellingLoc() used in case the range corresponds to a
    // macro/preprocessed source.
    auto start_loc = sm.getSpellingLoc(range.getBegin());
    auto last_token_loc = sm.getSpellingLoc(range.getEnd());
    auto end_loc = clang::Lexer::getLocForEndOfToken(last_token_loc, 0, sm, lo);
    auto printable_range = clang::SourceRange{start_loc, end_loc};
    return get_source_text_raw(printable_range, sm);
}

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::class_diagram::model::diagram &diagram,
    const clanguml::config::class_diagram &config)
    : source_manager_{sm}
    , diagram_{diagram}
    , config_{config}
{
}

bool translation_unit_visitor::VisitNamespaceDecl(clang::NamespaceDecl *ns)
{
    auto package_path = namespace_{ns->getQualifiedNameAsString()};
    auto package_parent = package_path;

    std::string name;
    if (!package_path.is_empty())
        name = package_path.name();

    if (!package_parent.is_empty())
        package_parent.pop_back();

    const auto usn = config().using_namespace();

    auto p = std::make_unique<common::model::package>(usn);
    package_path = package_path.relative_to(usn);

    p->set_name(name);
    p->set_namespace(package_parent);

    if (diagram().should_include(*p)) {
        process_comment(*ns, *p);
        set_source_location(*ns, *p);

        p->set_style(p->style_spec());

        for (const auto *attr : ns->attrs()) {
            if (attr->getKind() == clang::attr::Kind::Deprecated) {
                p->set_deprecated(true);
                break;
            }
        }

        if (!p->skip()) {
            diagram().add_package(std::move(p));
        }
    }

    return true;
}

bool translation_unit_visitor::VisitEnumDecl(clang::EnumDecl *enm)
{
    auto e_ptr = std::make_unique<enum_>(config_.using_namespace());
    auto &e = *e_ptr;

    std::string qualified_name = enm->getQualifiedNameAsString();
    namespace_ ns{qualified_name};
    ns.pop_back();
    e.set_name(enm->getNameAsString());
    e.set_namespace(ns);
    e.set_id(enm->getID());

    process_comment(*enm, e);
    set_source_location(*enm, e);

    if (e.skip())
        return true;

    e.set_style(e.style_spec());

    for (const auto &ev : enm->enumerators()) {
        e.constants().push_back(ev->getNameAsString());
    }

    if (enm->getParent()->isRecord()) {
        process_record_containment(*enm, e);
    }

    auto namespace_declaration = detail::get_enclosing_namespace(enm);
    if (namespace_declaration.has_value()) {
        e.set_namespace(namespace_declaration.value());
    }

    if (diagram().should_include(qualified_name))
        diagram().add_enum(std::move(e_ptr));

    return true;
}

bool translation_unit_visitor::VisitClassTemplateDecl(
    clang::ClassTemplateDecl *cls)
{
    if (source_manager_.isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    auto c_ptr = process_class_declaration(cls->getTemplatedDecl());

    // Override the id with the template id, for now we don't care about the
    // underlying templated class id
    c_ptr->set_id(cls->getID());

    process_template_parameters(*cls, *c_ptr);

    if (diagram_.should_include(*c_ptr)) {
        LOG_DBG("Adding class {} with id {}", c_ptr->full_name(), c_ptr->id());
        diagram_.add_class(std::move(c_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitCXXRecordDecl(clang::CXXRecordDecl *cls)
{
    // Skip system headers
    if (source_manager_.isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    // Templated records are handled by VisitClassTemplateDecl()
    if (cls->isTemplated())
        return true;

    // Skip forward declarations
    if (!cls->isCompleteDefinition())
        return true;

    // Check if the class was already processed within VisitClassTemplateDecl()
    if (diagram_.has_element(cls->getID()))
        return true;

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass())
        return true;

    auto c_ptr = process_class_declaration(cls);

    if (!c_ptr)
        return true;

    if (diagram_.should_include(*c_ptr)) {
        diagram_.add_class(std::move(c_ptr));
    }

    return true;
}

std::unique_ptr<class_> translation_unit_visitor::process_class_declaration(
    clang::CXXRecordDecl *cls)
{
    auto c_ptr{std::make_unique<class_>(config_.using_namespace())};
    auto &c = *c_ptr;

    namespace_ ns{cls->getQualifiedNameAsString()};
    ns.pop_back();
    c.set_name(cls->getNameAsString());
    c.set_namespace(ns);
    c.set_id(cls->getID());

    c.is_struct(cls->isStruct());

    process_comment(*cls, c);
    set_source_location(*cls, c);

    if (c.skip())
        return {};

    c.set_style(c.style_spec());

    // Process class child entities
    process_class_children(cls, c);

    // Process class bases
    process_class_bases(cls, c);

    if (cls->getParent()->isRecord()) {
        process_record_containment(*cls, c);
    }

    return c_ptr;
}

bool translation_unit_visitor::process_template_parameters(
    const clang::ClassTemplateDecl &template_declaration, class_ &c)
{
    LOG_DBG("Processing class {} template parameters...",
        template_declaration.getQualifiedNameAsString());

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
            LOG_DBG("============= UNKNOWN TEMPLATE PARAMETER TYPE:");
            parameter->dump();
        }
    }

    return false;
}

void translation_unit_visitor::process_record_containment(
    const clang::TagDecl &record,
    clanguml::common::model::element &element) const
{
    assert(record.getParent()->isRecord());

    const auto *parent = record.getParent()->getOuterLexicalRecordContext();
    auto parent_name =
        static_cast<const clang::RecordDecl *>(record.getParent())
            ->getQualifiedNameAsString();

    auto namespace_declaration = detail::get_enclosing_namespace(parent);
    if (namespace_declaration.has_value()) {
        element.set_namespace(namespace_declaration.value());
    }

    const auto id =
        static_cast<const clang::RecordDecl *>(record.getParent())->getID();

    element.add_relationship({relationship_t::kContainment, id});
}

void translation_unit_visitor::process_class_bases(
    const clang::CXXRecordDecl *cls, class_ &c) const
{
    for (auto &base : cls->bases()) {
        class_parent cp;
        auto name_and_ns = common::model::namespace_{
            to_string(base.getType(), cls->getASTContext())};

        cp.set_name(name_and_ns.to_string());
        cp.set_id(
            base.getType()->getAs<clang::RecordType>()->getDecl()->getID());
        cp.is_virtual(base.isVirtual());

        cp.set_access(
            detail::access_specifier_to_access_t(base.getAccessSpecifier()));

        LOG_DBG("Found base class {} for class {}", cp.name(), c.name());

        c.add_parent(std::move(cp));
    }
}

void translation_unit_visitor::process_class_children(
    const clang::CXXRecordDecl *cls, class_ &c)
{
    assert(cls != nullptr);

    // Iterate over class methods (both regular and static)
    for (const auto *method : cls->methods()) {
        if (method != nullptr) {

            process_method(*method, c);
        }
    }

    // Iterate over class template methods
    for (auto const *decl_iterator :
        clang::dyn_cast_or_null<clang::DeclContext>(cls)->decls()) {
        auto const *method_template =
            llvm::dyn_cast_or_null<clang::FunctionTemplateDecl>(decl_iterator);
        if (method_template == nullptr)
            continue;

        process_template_method(*method_template, c);
    }

    // Iterate over regular class fields
    for (const auto *field : cls->fields()) {
        if (field != nullptr)
            process_field(*field, c);
    }

    // Static fields have to be processed by iterating over variable
    // declarations
    for (const auto *decl : cls->decls()) {
        if (decl->getKind() == clang::Decl::Var) {
            const clang::VarDecl *variable_declaration{
                dynamic_cast<const clang::VarDecl *>(decl)};
            if (variable_declaration &&
                variable_declaration->isStaticDataMember()) {
                process_static_field(*variable_declaration, c);
            }
        }
    }

    for (const auto *friend_declaration : cls->friends()) {
        process_friend(*friend_declaration, c);
    }
}

void translation_unit_visitor::process_friend(
    const clang::FriendDecl &f, class_ &c)
{
    if (const auto *friend_type_info = f.getFriendType()) {
        const auto friend_type = friend_type_info->getType();
        if (friend_type->getAs<clang::TemplateSpecializationType>() !=
            nullptr) {
            // TODO: handle template friend
        }
        else if (friend_type->getAs<clang::RecordType>()) {
            const auto friend_type_name =
                friend_type->getAsRecordDecl()->getQualifiedNameAsString();
            if (diagram().should_include(friend_type_name)) {
                relationship r{relationship_t::kFriendship,
                    friend_type->getAsRecordDecl()->getID(),
                    detail::access_specifier_to_access_t(f.getAccess()),
                    "<<friend>>"};

                c.add_relationship(std::move(r));
            }
        }
    }
}

void translation_unit_visitor::process_method(
    const clang::CXXMethodDecl &mf, class_ &c)
{
    // TODO: For now skip implicitly default methods
    //       in the future, add config option to choose
    if (mf.isDefaulted() && !mf.isExplicitlyDefaulted())
        return;

    class_method method{detail::access_specifier_to_access_t(mf.getAccess()),
        util::trim(mf.getNameAsString()), mf.getReturnType().getAsString()};

    method.is_pure_virtual(mf.isPure());
    method.is_virtual(mf.isVirtual());
    method.is_const(mf.isConst());
    method.is_defaulted(mf.isDefaulted());
    method.is_static(mf.isStatic());

    process_comment(mf, method);

    if (method.skip())
        return;

    for (const auto *param : mf.parameters()) {
        if (param != nullptr)
            process_function_parameter(*param, method, c);
    }

    LOG_DBG("Adding method: {}", method.name());

    c.add_method(std::move(method));
}

void translation_unit_visitor::process_template_method(
    const clang::FunctionTemplateDecl &mf, class_ &c)
{
    // TODO: For now skip implicitly default methods
    //       in the future, add config option to choose
    if (mf.getTemplatedDecl()->isDefaulted() &&
        !mf.getTemplatedDecl()->isExplicitlyDefaulted())
        return;

    class_method method{detail::access_specifier_to_access_t(mf.getAccess()),
        util::trim(mf.getNameAsString()),
        mf.getTemplatedDecl()->getReturnType().getAsString()};

    method.is_pure_virtual(mf.getTemplatedDecl()->isPure());
    method.is_virtual(false);
    method.is_const(false);
    method.is_defaulted(mf.getTemplatedDecl()->isDefaulted());
    method.is_static(mf.getTemplatedDecl()->isStatic());

    process_comment(mf, method);

    if (method.skip())
        return;

    for (const auto *param : mf.getTemplatedDecl()->parameters()) {
        if (param != nullptr)
            process_function_parameter(*param, method, c);
    }

    LOG_DBG("Adding method: {}", method.name());

    c.add_method(std::move(method));
}

bool translation_unit_visitor::find_relationships(const clang::QualType &type,
    found_relationships_t &relationships,
    clanguml::common::model::relationship_t relationship_hint)
{
    bool result = false;
    std::string type_name = type.getAsString();
    (void)type_name;

    if (type->isPointerType()) {
        relationship_hint = relationship_t::kAssociation;
        find_relationships(
            type->getPointeeType(), relationships, relationship_hint);
    }
    else if (type->isRValueReferenceType()) {
        relationship_hint = relationship_t::kAggregation;
        find_relationships(
            type.getNonReferenceType(), relationships, relationship_hint);
    }
    else if (type->isLValueReferenceType()) {
        relationship_hint = relationship_t::kAssociation;
        find_relationships(
            type.getNonReferenceType(), relationships, relationship_hint);
    }
    else if (type->isArrayType()) {
        find_relationships(type->getAsArrayTypeUnsafe()->getElementType(),
            relationships, relationship_t::kAggregation);
    }
    else if (type->isRecordType()) {
        if (type_name.find("std::shared_ptr") == 0)
            relationship_hint = relationship_t::kAssociation;
        if (type_name.find("std::weak_ptr") == 0)
            relationship_hint = relationship_t::kAssociation;

        const auto *type_instantiation_decl =
            type->getAs<clang::TemplateSpecializationType>();

        if (type_instantiation_decl != nullptr) {
            for (const auto &template_argument : *type_instantiation_decl) {
                if (template_argument.getKind() ==
                    clang::TemplateArgument::ArgKind::Type)
                    result = find_relationships(template_argument.getAsType(),
                        relationships, relationship_hint);
            }
        }
        else {
            const auto target_id = type->getAsCXXRecordDecl()->getID();
            relationships.emplace_back(target_id, relationship_hint);
            result = true;
        }
    }

    return result;
}

void translation_unit_visitor::process_function_parameter(
    const clang::ParmVarDecl &p, class_method &method, class_ &c,
    const std::set<std::string> &template_parameter_names)
{
    method_parameter parameter;
    parameter.set_name(p.getNameAsString());

    process_comment(p, parameter);

    if (parameter.skip())
        return;

    parameter.set_type(p.getType().getAsString());

    if (p.hasDefaultArg()) {
        const auto *default_arg = p.getDefaultArg();
        if (default_arg != nullptr) {
            auto default_arg_str =
                default_arg->getSourceRange().printToString(source_manager_);
            parameter.set_default_value(default_arg_str);
        }
    }

    if (!parameter.skip_relationship()) {
        // find relationship for the type
        found_relationships_t relationships;

        find_relationships(
            p.getType(), relationships, relationship_t::kDependency);

        for (const auto &[type_element_id, relationship_type] : relationships) {
            if (/*diagram().has_element(type_element_id) &&*/
                (relationship_type != relationship_t::kNone)) {
                relationship r{relationship_t::kDependency, type_element_id};

                LOG_DBG("Adding field relationship {} {} {} : {}",
                    r.destination(),
                    clanguml::common::model::to_string(r.type()), c.full_name(),
                    r.label());

                c.add_relationship(std::move(r));
            }
        }

        // Also consider the container itself if it is a template instantiation
        // it's arguments could count as reference to relevant types
        auto underlying_type = p.getType();
        if (underlying_type->isReferenceType())
            underlying_type = underlying_type.getNonReferenceType();
        if (underlying_type->isPointerType())
            underlying_type = underlying_type->getPointeeType();

        if (underlying_type->getAs<clang::TemplateSpecializationType>() !=
            nullptr) {
            process_function_parameter_find_relationships_in_template(c,
                template_parameter_names,
                *underlying_type->getAs<clang::TemplateSpecializationType>());
        }
    }

    method.add_parameter(std::move(parameter));
}

void translation_unit_visitor::
    process_function_parameter_find_relationships_in_template(class_ &c,
        const std::set<std::string> &template_parameter_names,
        const clang::TemplateSpecializationType &template_instantiation_type)
{
    const auto template_field_decl_name =
        template_instantiation_type.getTemplateName()
            .getAsTemplateDecl()
            ->getQualifiedNameAsString();

    if (diagram().should_include(template_field_decl_name)) {
        if (template_instantiation_type.isDependentType()) {
            relationship r{relationship_t::kDependency,
                template_instantiation_type.getTemplateName()
                    .getAsTemplateDecl()
                    ->getID()};

            c.add_relationship(std::move(r));
        }
        else {
            auto template_specialization_ptr =
                build_template_instantiation(template_instantiation_type);

            if (template_specialization_ptr) {
                relationship r{relationship_t::kDependency,
                    template_specialization_ptr->id()};

                if (!diagram().has_element(template_specialization_ptr->id()))
                    diagram().add_class(std::move(template_specialization_ptr));

                c.add_relationship(std::move(r));
            }
        }
    }
}

void translation_unit_visitor::add_relationships(class_ &c,
    const found_relationships_t &relationships, access_t access,
    std::optional<std::string> label)
{
    for (const auto &[target, relationship_type] : relationships) {
        if (diagram().has_element(target) &&
            (relationship_type != relationship_t::kNone) /*&&
            (target != c.name_and_ns())*/) {
            relationship r{relationship_type, target};
            if (label)
                r.set_label(label.value());
            r.set_access(access);

            LOG_DBG("Adding field relationship {} {} {} : {}", r.destination(),
                clanguml::common::model::to_string(r.type()), c.full_name(),
                r.label());

            c.add_relationship(std::move(r));
        }
    }
}

void translation_unit_visitor::process_static_field(
    const clang::VarDecl &field_declaration, class_ &c)
{
    const auto field_type = field_declaration.getType();
    auto type_name = to_string(field_type, field_declaration.getASTContext());
    if (type_name.empty())
        type_name = "<<anonymous>>";

    class_member field{
        detail::access_specifier_to_access_t(field_declaration.getAccess()),
        field_declaration.getNameAsString(), type_name};
    field.is_static(true);

    process_comment(field_declaration, field);
    set_source_location(field_declaration, field);

    if (field.skip())
        return;

    if (!field.skip_relationship()) {
        found_relationships_t relationships;

        // find relationship for the type
        find_relationships(field_declaration.getType(), relationships,
            relationship_t::kAssociation);

        add_relationships(c, relationships,
            detail::access_specifier_to_access_t(field_declaration.getAccess()),
            field_declaration.getNameAsString());
    }

    c.add_member(std::move(field));
}

std::unique_ptr<class_> translation_unit_visitor::build_template_instantiation(
    const clang::TemplateSpecializationType &template_type,
    std::optional<clanguml::class_diagram::model::class_ *> parent)
{
    // TODO: Make sure we only build instantiation once

    //
    // Create class_ instance to hold the template instantiation
    //
    auto template_instantiation_ptr =
        std::make_unique<class_>(config_.using_namespace());
    auto &template_instantiation = *template_instantiation_ptr;
    std::string full_template_specialization_name = to_string(
        template_type.desugar(),
        template_type.getTemplateName().getAsTemplateDecl()->getASTContext());

    const auto *template_decl{
        template_type.getTemplateName().getAsTemplateDecl()};

    auto qualified_name = template_decl->getQualifiedNameAsString();

    namespace_ ns{qualified_name};
    ns.pop_back();
    template_instantiation.set_name(template_decl->getNameAsString());
    template_instantiation.set_namespace(ns);
    template_instantiation.set_id(template_decl->getID() +
        (std::hash<std::string>{}(full_template_specialization_name) >> 4));

    for (const auto &arg : template_type) {
        const auto argument_kind = arg.getKind();
        if (argument_kind == clang::TemplateArgument::ArgKind::Type) {
            template_parameter argument;
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

                auto nested_template_instantiation =
                    build_template_instantiation(
                        *arg.getAsType()
                             ->getAs<clang::TemplateSpecializationType>(),
                        parent);

                for (const auto &t : nested_template_instantiation->templates())
                    argument.add_template_param(t);

                // Check if this template should be simplified (e.g. system
                // template aliases such as 'std:basic_string<char>' should be
                // simply 'std::string')
                simplify_system_template(argument,
                    argument.to_string(config().using_namespace(), false));
            }
            else {
                argument.set_name(
                    to_string(arg.getAsType(), template_decl->getASTContext()));
            }

            template_instantiation.add_template(std::move(argument));
        }
        else if (argument_kind == clang::TemplateArgument::ArgKind::Integral) {
            template_parameter argument;
            argument.is_template_parameter(false);
            argument.set_type(
                std::to_string(arg.getAsIntegral().getExtValue()));
            template_instantiation.add_template(std::move(argument));
        }
        else if (argument_kind ==
            clang::TemplateArgument::ArgKind::Expression) {
            template_parameter argument;
            argument.is_template_parameter(false);
            argument.set_type(get_source_text(
                arg.getAsExpr()->getSourceRange(), source_manager_));
            template_instantiation.add_template(std::move(argument));
        }
        else {
            LOG_ERROR("UNSUPPORTED ARGUMENT KIND FOR ARG {}", arg.getKind());
        }
    }

    if (diagram().has_element(
            template_type.getTemplateName().getAsTemplateDecl()->getID())) {
        template_instantiation.add_relationship({relationship_t::kInstantiation,
            template_type.getTemplateName().getAsTemplateDecl()->getID()});
    }

    return template_instantiation_ptr;
}

void translation_unit_visitor::process_field(
    const clang::FieldDecl &field_declaration, class_ &c)
{
    auto relationship_hint = relationship_t::kAggregation;
    //    bool template_instantiation_added_as_aggregation{false};
    auto field_type = field_declaration.getType();
    const auto field_name = field_declaration.getNameAsString();
    auto type_name = to_string(field_type, field_declaration.getASTContext());
    if (type_name.empty())
        type_name = "<<anonymous>>";

    class_member field{
        detail::access_specifier_to_access_t(field_declaration.getAccess()),
        field_name, type_name};

    process_comment(field_declaration, field);
    set_source_location(field_declaration, field);

    if (field.skip())
        return;

    if (field_type->isPointerType()) {
        relationship_hint = relationship_t::kAssociation;
        field_type = field_type->getPointeeType();
    }
    else if (field_type->isLValueReferenceType()) {
        relationship_hint = relationship_t::kAssociation;
        field_type = field_type.getNonReferenceType();
    }
    else if (field_type->isRValueReferenceType()) {
        field_type = field_type.getNonReferenceType();
    }

    const auto *template_field_type =
        field_type->getAs<clang::TemplateSpecializationType>();

    if (template_field_type != nullptr) {
        const auto template_field_decl_name =
            template_field_type->getTemplateName()
                .getAsTemplateDecl()
                ->getQualifiedNameAsString();
        if (diagram().should_include(template_field_decl_name)) {
            auto template_specialization_ptr = build_template_instantiation(
                *field_type->getAs<clang::TemplateSpecializationType>());

            relationship r{
                relationship_hint, template_specialization_ptr->id()};
            r.set_label(field_declaration.getNameAsString());
            r.set_access(detail::access_specifier_to_access_t(
                field_declaration.getAccess()));

            diagram().add_class(std::move(template_specialization_ptr));

            LOG_DBG("ADDED TEMPLATE SPECIALIZATION TO DIAGRAM");

            c.add_relationship(std::move(r));
        }
        else {
            if (!field.skip_relationship()) {
                found_relationships_t relationships;
                // find relationship for the type
                find_relationships(
                    field_type, relationships, relationship_hint);

                add_relationships(c, relationships,
                    detail::access_specifier_to_access_t(
                        field_declaration.getAccess()),
                    field_declaration.getNameAsString());
            }
        }
    }
    else {
        if (!field.skip_relationship()) {
            found_relationships_t relationships;
            // find relationship for the type
            find_relationships(field_type, relationships, relationship_hint);

            add_relationships(c, relationships,
                detail::access_specifier_to_access_t(
                    field_declaration.getAccess()),
                field_declaration.getNameAsString());
        }
    }

    c.add_member(std::move(field));
}

void translation_unit_visitor::set_source_location(
    const clang::Decl &decl, clanguml::common::model::source_location &element)
{
    if (decl.getLocation().isValid()) {
        element.set_file(source_manager_.getFilename(decl.getLocation()).str());
        element.set_line(
            source_manager_.getSpellingLineNumber(decl.getLocation()));
    }
}

bool translation_unit_visitor::simplify_system_template(
    template_parameter &ct, const std::string &full_name)
{
    if (config().template_aliases().count(full_name) > 0) {
        ct.set_name(config().template_aliases().at(full_name));
        ct.clear_params();
        return true;
    }
    else
        return false;
}
}
