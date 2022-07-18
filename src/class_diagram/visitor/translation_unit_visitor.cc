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

        auto canonical_qualified_name =
            canonical_type_name.substr(0, canonical_type_name.find('<'));
        auto result_template_arguments = result.substr(result.find('<'));

        result = canonical_qualified_name + result_template_arguments;
    }

    return result;
}

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::class_diagram::model::diagram &diagram,
    const clanguml::config::class_diagram &config)
    : source_manager_{sm}
    , diagram_{diagram}
    , config_{config}
{
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

    c_ptr->set_id(cls->getID());

    if (!c_ptr)
        return true;

    process_template_parameters(*cls, *c_ptr);

    if (diagram_.should_include(*c_ptr)) {
        diagram_.add_class(std::move(c_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitCXXRecordDecl(clang::CXXRecordDecl *cls)
{
    if (source_manager_.isInSystemHeader(cls->getSourceRange().getBegin()))
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
    auto c_ptr = std::make_unique<class_>(config_.using_namespace());
    auto &c = *c_ptr;

    c.is_struct(cls->isStruct());

    namespace_ ns{cls->getQualifiedNameAsString()};
    ns.pop_back();
    c.set_name(cls->getNameAsString());
    c.set_namespace(ns);
    c.set_id(cls->getID());

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
            ct.set_type("");
            ct.is_template_parameter(false);
            ct.set_name(template_nontype_parameter->getNameAsString());
            ct.set_default_value("");
            ct.is_variadic(template_nontype_parameter->isParameterPack());

            c.add_template(std::move(ct));
        }
    }

    /*
    auto scope = cppast::cpp_scope_name(type_safe::ref(cls));
    // Even if this is a template the scope.is_templated() returns
    // false when the template parameter list is empty
    if (scope.is_templated()) {
        process_scope_template_parameters(c, scope);
    }
    else {
        LOG_DBG("Class {} is templated but it's scope {} is not - "
                "probably this is a specialization",
            cls.name(), scope.name());

        // Add specialization arguments
        if (tspec) {
            if (!tspec.value().arguments_exposed()) {
                process_unexposed_template_specialization_parameters(tspec, c);
            }
            else {
                process_exposed_template_specialization_parameters(tspec, c);
            }
        }
        else {
            LOG_DBG("Skipping template class declaration which has only "
                    "unexposed arguments but no tspec provided");
            return true;
        }
    }
     */

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
        if (method != nullptr)
            process_method(*method, c);
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
    else if (type->isClassType()) {
        const auto *type_instantiation_decl =
            type->getAs<clang::TemplateSpecializationType>();

        if (type_instantiation_decl != nullptr) {
            for (const auto &template_argument : *type_instantiation_decl) {
                result = find_relationships(template_argument.getAsType(),
                    relationships, relationship_hint);
            }
        }
        else {
            relationships.emplace_back(
                type->getAsCXXRecordDecl()->getID(), relationship_hint);
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

    /*
        if (!parameter.skip_relationship()) {
            // find relationship for the type
            std::vector<std::pair<std::string, relationship_t>> relationships;

            find_relationships(cppast::remove_cv(param.type()), relationships,
                relationship_t::kDependency);

            for (const auto &[type, relationship_type] : relationships) {
                if (type.empty())
                    continue;

                auto [type_ns, type_name] = cx::util::split_ns(type);
                if (ctx.diagram().should_include(type_ns, type_name) &&
                    (relationship_type != relationship_t::kNone) &&
                    (type != c.name_and_ns())) {
                    relationship r{relationship_t::kDependency, type};

                    LOG_DBG("Adding field relationship {} {} {} : {}",
                        r.destination(),
                        clanguml::common::model::to_string(r.type()),
       c.full_name(), r.label());

                    c.add_relationship(std::move(r));
                }
            }

            // Also consider the container itself if it is a template
       instantiation
            // it's arguments could count as reference to relevant types
            const auto &t =
       cppast::remove_cv(cx::util::unreferenced(param.type())); if (t.kind() ==
       cppast::cpp_type_kind::template_instantiation_t) {
                process_function_parameter_find_relationships_in_template(
                    c, template_parameter_names, t);
            }
        }
    */
    method.add_parameter(std::move(parameter));
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
    //
    // Create class_ instance to hold the template instantiation
    //
    auto template_instantiation_ptr =
        std::make_unique<class_>(config_.using_namespace());
    auto &template_instantiation = *template_instantiation_ptr;
    std::string full_template_specialization_name =
        to_string(template_type.desugar(),
            template_type.getAsCXXRecordDecl()->getASTContext());

    const auto *template_record{
        template_type.getTemplateName().getAsTemplateDecl()};
    std::string qualified_name = template_record->getQualifiedNameAsString();
    namespace_ ns{qualified_name};
    ns.pop_back();
    template_instantiation.set_name(template_record->getNameAsString());
    template_instantiation.set_namespace(ns);
    template_instantiation.set_id(template_record->getID() +
        std::hash<std::string>{}(full_template_specialization_name));

    for (const auto &arg : template_type) {
        auto argument_kind = arg.getKind();
        if (argument_kind == clang::TemplateArgument::ArgKind::Type) {
            const auto *argument_record_decl =
                arg.getAsType()->getAsRecordDecl();
            template_parameter argument;
            argument.is_template_parameter(false);
            argument.set_name(to_string(
                arg.getAsType(), argument_record_decl->getASTContext()));

            template_instantiation.add_template(std::move(argument));

            template_instantiation.add_relationship(
                {relationship_t::kInstantiation, template_record->getID()});
        }
    }

    return template_instantiation_ptr;
}

void translation_unit_visitor::process_field(
    const clang::FieldDecl &field_declaration, class_ &c)
{
    //    bool template_instantiation_added_as_aggregation{false};
    const auto field_type = field_declaration.getType();
    auto type_name = to_string(field_type, field_declaration.getASTContext());
    if (type_name.empty())
        type_name = "<<anonymous>>";

    class_member field{
        detail::access_specifier_to_access_t(field_declaration.getAccess()),
        field_declaration.getNameAsString(), type_name};

    if (field.name() == "e") {
        LOG_DBG("EEEEEEEEE");
    }

    process_comment(field_declaration, field);
    set_source_location(field_declaration, field);

    if (field.skip())
        return;

    if (!field.skip_relationship()) {
        found_relationships_t relationships;
        // find relationship for the type
        find_relationships(field_declaration.getType(), relationships,
            relationship_t::kAggregation);

        add_relationships(c, relationships,
            detail::access_specifier_to_access_t(field_declaration.getAccess()),
            field_declaration.getNameAsString());
    }

    if (field_type->getAs<clang::TemplateSpecializationType>() &&
        diagram().should_include(type_name)) {

        auto template_specialization_ptr = build_template_instantiation(
            *field_type->getAs<clang::TemplateSpecializationType>());

        diagram().add_class(std::move(template_specialization_ptr));

        LOG_DBG("ADDED TEMPLATE SPECIALIZATION TO DIAGRAM");
    }

    /*
    if (field_type->getAsCXXRecordDecl()) {

           ElaboratedType 0x5555587d0850 'std::vector<A *>' sugar
                `-TemplateSpecializationType 0x5555587d0810 'vector<class
           clanguml::t00002::A *>' sugar vector
                  |-TemplateArgument type 'class clanguml::t00002::A *'
                  | `-PointerType 0x5555587ce870 'class clanguml::t00002::A *'
                  |   `-RecordType 0x5555587ce700 'class clanguml::t00002::A'
                  |     `-CXXRecord 0x5555587ce668 'A'
                  `-RecordType 0x5555587d07f0 'class std::vector<class
           clanguml::t00002::A *>'
                    `-ClassTemplateSpecialization 0x5555587d06f0 'vector'
         */
    //    field_type->dump();
    //
    //    const auto *type_instantiation_decl =
    //        field_type->getAs<clang::TemplateSpecializationType>();
    //    //->desugar()
    //    //->getAs<clang::RecordType>()
    //    //->getDecl();
    //
    //    std::string type_decl_str =
    //        field_type->getAsCXXRecordDecl()->getQualifiedNameAsString();
    //
    //    (void)type_decl_str;
    //
    //    for (const auto &f : *type_instantiation_decl) {
    //        std::cout << " [[[===================== \n";
    //        f.dump();
    //        std::cout << " ]]]===================== \n";
    //        if (f.getAsType()->isPointerType()) {
    //            f.getAsType()->getPointeeType().dump();
    //            if (f.getAsType()->getPointeeType()->isClassType()) {
    //                const auto t =
    //                    f.getAsType()->getPointeeType()->getAsCXXRecordDecl();
    //                std::string tt = t->getQualifiedNameAsString();
    //                (void)tt;
    //            }
    //        }
    //    }

    /*
    if (clang::isTemplateInstantiation(
            type_decl->getTemplateSpecializationKind())) {
        for (const auto *template_param :
            *type_decl->getTemplateParameterList(0)) {
            template_param->getDescribedTemplate()->getCanonicalDecl()->
        }
        // for(const auto* template_param : type_decl->templ
        //         type_decl->getTemplateInstantiationPattern()
    }
     */
    //}

    //    if (field.isTemplated()) {
    //        for (const auto *param : *field.getTemplateParameterList(0)) {
    //            std::string param_type_str =
    //            param->getQualifiedNameAsString(); (void)param_type_str;
    //        }
    //    }
    /*
        const auto &tr = cx::util::unreferenced(cppast::remove_cv(mv.type()));

        auto tr_declaration = cppast::to_string(tr);

        LOG_DBG("Processing field {} with unreferenced type of kind {}",
       mv.name(), cppast::to_string(tr.kind()));

        if (tr.kind() == cppast::cpp_type_kind::builtin_t) {
            LOG_DBG("Builtin type found for field: {}", m.name());
        }
        else if (tr.kind() == cppast::cpp_type_kind::user_defined_t) {
            LOG_DBG("Processing user defined type field {} {}",
                cppast::to_string(tr), mv.name());
            if (resolve_alias(tr).kind() ==
                cppast::cpp_type_kind::template_instantiation_t)
                template_instantiation_added_as_aggregation =
                    process_field_with_template_instantiation(mv, tr, c, m, as);
        }
        else if (tr.kind() == cppast::cpp_type_kind::template_instantiation_t) {
            // This can be either template instantiation or an alias template
            // instantiation
            template_instantiation_added_as_aggregation =
                process_field_with_template_instantiation(mv, tr, c, m, as);
        }
        else if (tr.kind() == cppast::cpp_type_kind::unexposed_t) {
            LOG_DBG(
                "Processing field with unexposed type {}",
       cppast::to_string(tr));
            // TODO
        }

        //
        // Try to find relationships in the type of the member, unless it has
        // been already added as part of template processing or it is marked
        // to be skipped in the comment
        //
        if (!m.skip_relationship() &&
            !template_instantiation_added_as_aggregation &&
            (tr.kind() != cppast::cpp_type_kind::builtin_t) &&
            (tr.kind() != cppast::cpp_type_kind::template_parameter_t)) {
            found_relationships_t relationships;

            const auto &unaliased_type = resolve_alias(mv.type());
            find_relationships(unaliased_type, relationships);

            for (const auto &[type, relationship_type] : relationships) {
                if (relationship_type != relationship_t::kNone) {
                    relationship r{relationship_type, type, m.access(),
       m.name()}; r.set_style(m.style_spec());

                    auto [decorator_rtype, decorator_rmult] =
       m.get_relationship(); if (decorator_rtype != relationship_t::kNone) {
                        r.set_type(decorator_rtype);
                        auto mult = util::split(decorator_rmult, ":", false);
                        if (mult.size() == 2) {
                            r.set_multiplicity_source(mult[0]);
                            r.set_multiplicity_destination(mult[1]);
                        }
                    }

                    LOG_DBG("Adding field relationship {} {} {} : {}",
                        r.destination(),
                        clanguml::common::model::to_string(r.type()),
       c.full_name(), r.label());

                    c.add_relationship(std::move(r));
                }
            }
        }
        */
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

}
