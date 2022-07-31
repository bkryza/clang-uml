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
#include "common/clang_utils.h"
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

    return namespace_{common::get_qualified_name(*namespace_declaration)};
}
}

std::string to_string(const clang::QualType &type, const clang::ASTContext &ctx,
    bool try_canonical = true)
{
    const clang::PrintingPolicy print_policy(ctx.getLangOpts());

    auto result{type.getAsString(print_policy)};

    if (try_canonical && result.find('<') != std::string::npos) {
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

    util::replace_all(result, ", ", ",");

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
    assert(ns != nullptr);

    if (ns->isAnonymousNamespace() || ns->isInline())
        return true;

    auto package_path = namespace_{common::get_qualified_name(*ns)};
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
    p->set_id(common::to_id(*ns));

    if (diagram().should_include(*p) && !diagram().get(p->id())) {
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
    assert(enm != nullptr);

    // Anonymous enum values should be rendered as class fields
    // with type enum
    if (enm->getNameAsString().empty())
        return true;

    auto e_ptr = std::make_unique<enum_>(config_.using_namespace());
    auto &e = *e_ptr;

    std::string qualified_name = common::get_qualified_name(*enm);
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

bool translation_unit_visitor::VisitClassTemplateSpecializationDecl(
    clang::ClassTemplateSpecializationDecl *cls)
{
    if (source_manager_.isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    // Skip forward declarations
    if (!cls->isCompleteDefinition()) {
        // Register this forward declaration in case there is no complete
        // definition (see t00036)
        return true;
    }
    else
        // Check if the class was already processed within
        // VisitClassTemplateDecl()
        if (diagram_.has_element(cls->getID()))
        return true;

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass())
        return true;

    auto template_specialization_ptr = process_template_specialization(cls);

    if (!template_specialization_ptr)
        return true;

    auto &template_specialization = *template_specialization_ptr;

    process_template_specialization_children(cls, template_specialization);

    // Process template specialization bases
    process_class_bases(cls, template_specialization);

    template_specialization.add_relationship({relationship_t::kInstantiation,
        cls->getSpecializedTemplate()->getID()});

    if (diagram_.should_include(template_specialization)) {
        LOG_DBG("Adding class template specialization {} with id {}",
            template_specialization.full_name(false),
            template_specialization.id());

        diagram_.add_class(std::move(template_specialization_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitTypeAliasTemplateDecl(
    clang::TypeAliasTemplateDecl *cls)
{
    if (source_manager_.isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    auto *template_type_specialization_ptr =
        cls->getTemplatedDecl()
            ->getUnderlyingType()
            ->getAs<clang::TemplateSpecializationType>();

    if (template_type_specialization_ptr == nullptr)
        return true;

    auto template_specialization_ptr =
        build_template_instantiation(*template_type_specialization_ptr);

    if (!template_specialization_ptr)
        return true;

    if (diagram_.should_include(*template_specialization_ptr)) {
        LOG_DBG("Adding class {} with id {}",
            template_specialization_ptr->full_name(),
            template_specialization_ptr->id());

        diagram_.add_class(std::move(template_specialization_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitClassTemplateDecl(
    clang::ClassTemplateDecl *cls)
{
    if (source_manager_.isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    // Skip forward declarations
    if (!cls->getTemplatedDecl()->isCompleteDefinition())
        return true;

    auto c_ptr = create_class_declaration(cls->getTemplatedDecl());

    if (!c_ptr)
        return true;

    // Override the id with the template id, for now we don't care about the
    // underlying templated class id
    c_ptr->set_id(cls->getID());

    auto id = c_ptr->id();

    process_template_parameters(*cls, *c_ptr);

    process_class_declaration(*cls->getTemplatedDecl(), *c_ptr);

    if (!cls->getTemplatedDecl()->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(c_ptr));
        return true;
    }
    else
        forward_declarations_.erase(id);

    if (diagram_.should_include(*c_ptr)) {
        LOG_DBG("Adding class template {} with id {}", c_ptr->full_name(),
            c_ptr->id());

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
    if (cls->isTemplated() || cls->isTemplateDecl() ||
        (clang::dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(cls) !=
            nullptr))
        return true;

    // Check if the class was already processed within VisitClassTemplateDecl()
    if (diagram_.has_element(cls->getID()))
        return true;

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass())
        return true;

    auto c_ptr = create_class_declaration(cls);

    if (!c_ptr)
        return true;

    auto &class_model = *c_ptr;

    process_class_declaration(*cls, class_model);

    auto id = class_model.id();
    if (!cls->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(c_ptr));
        return true;
    }
    else
        forward_declarations_.erase(id);

    if (diagram_.should_include(class_model)) {
        LOG_DBG("Adding class {} with id {}", class_model.full_name(),
            class_model.id());

        diagram_.add_class(std::move(c_ptr));
    }

    return true;
}

std::unique_ptr<class_> translation_unit_visitor::create_class_declaration(
    clang::CXXRecordDecl *cls)
{
    assert(cls != nullptr);

    auto c_ptr{std::make_unique<class_>(config_.using_namespace())};
    auto &c = *c_ptr;

    // TODO: refactor to method get_qualified_name()
    auto qualified_name = common::get_qualified_name(*cls);

    namespace_ ns{qualified_name};
    ns.pop_back();
    c.set_name(cls->getNameAsString());
    c.set_namespace(ns);
    c.set_id(common::to_id(*cls));

    c.is_struct(cls->isStruct());

    process_comment(*cls, c);
    set_source_location(*cls, c);

    if (c.skip())
        return {};

    c.set_style(c.style_spec());

    if (!cls->isCompleteDefinition())
        return c_ptr;

    return c_ptr;
}

void translation_unit_visitor::process_class_declaration(
    const clang::CXXRecordDecl &cls, class_ &c)
{
    // Process class child entities
    process_class_children(&cls, c);

    // Process class bases
    process_class_bases(&cls, c);

    if (cls.getParent()->isRecord()) {
        process_record_containment(cls, c);
    }
}

bool translation_unit_visitor::process_template_parameters(
    const clang::ClassTemplateDecl &template_declaration, class_ &c)
{
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

    const auto id = common::to_id(
        *static_cast<const clang::RecordDecl *>(record.getParent()));

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

        if (base.getType()->getAs<clang::RecordType>() != nullptr)
            cp.set_id(
                base.getType()->getAs<clang::RecordType>()->getDecl()->getID());
        else if (base.getType()->getAs<clang::TemplateSpecializationType>() !=
            nullptr) {
            cp.set_id(common::to_id(
                *base.getType()->getAs<clang::TemplateSpecializationType>()));
        }
        else
            // This could be a template parameter - we don't want it here
            continue;

        cp.is_virtual(base.isVirtual());

        cp.set_access(
            detail::access_specifier_to_access_t(base.getAccessSpecifier()));

        LOG_DBG("Found base class {} [{}] for class {}", cp.name(), cp.id(),
            c.name());

        c.add_parent(std::move(cp));
    }
}

void translation_unit_visitor::process_template_specialization_children(
    const clang::ClassTemplateSpecializationDecl *cls, class_ &c)
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
        else if (decl->getKind() == clang::Decl::Enum) {
            const auto *enum_decl =
                clang::dyn_cast_or_null<clang::EnumDecl>(decl);
            if (enum_decl == nullptr)
                continue;

            if (enum_decl->getNameAsString().empty()) {
                for (const auto *enum_const : enum_decl->enumerators()) {
                    class_member m{detail::access_specifier_to_access_t(
                                       enum_decl->getAccess()),
                        enum_const->getNameAsString(), "enum"};
                    c.add_member(std::move(m));
                }
            }
        }
    }

    for (const auto *friend_declaration : cls->friends()) {
        process_friend(*friend_declaration, c);
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
        else if (decl->getKind() == clang::Decl::Enum) {
            const auto *enum_decl =
                clang::dyn_cast_or_null<clang::EnumDecl>(decl);
            if (enum_decl == nullptr)
                continue;

            if (enum_decl->getNameAsString().empty()) {
                for (const auto *enum_const : enum_decl->enumerators()) {
                    class_member m{detail::access_specifier_to_access_t(
                                       enum_decl->getAccess()),
                        enum_const->getNameAsString(), "enum"};
                    c.add_member(std::move(m));
                }
            }
        }
    }

    if (cls->isCompleteDefinition())
        for (const auto *friend_declaration : cls->friends()) {
            if (friend_declaration != nullptr)
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
    bool result{false};

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
    else if (type->isEnumeralType()) {
        relationships.emplace_back(
            type->getAs<clang::EnumType>()->getDecl()->getID(),
            relationship_hint);
    }
    else if (type->isRecordType()) {
        const auto *type_instantiation_decl =
            type->getAs<clang::TemplateSpecializationType>();

        if (type_instantiation_decl != nullptr) {
            if (type_instantiation_decl->isTypeAlias())
                type_instantiation_decl =
                    type_instantiation_decl->getAliasedType()
                        ->getAs<clang::TemplateSpecializationType>();
        }

        if (type_instantiation_decl != nullptr) {
            for (const auto &template_argument : *type_instantiation_decl) {
                const auto template_argument_kind = template_argument.getKind();
                if (template_argument_kind ==
                    clang::TemplateArgument::ArgKind::Integral) {
                    // pass
                }
                else if (template_argument_kind ==
                    clang::TemplateArgument::ArgKind::Null) {
                    // pass
                }
                else if (template_argument_kind ==
                    clang::TemplateArgument::ArgKind::Expression) {
                    // pass
                }
                else if (template_argument.getKind() ==
                    clang::TemplateArgument::ArgKind::NullPtr) {
                    // pass
                }
                else if (template_argument_kind ==
                    clang::TemplateArgument::ArgKind::Template) {
                    // pass
                }
                else if (template_argument_kind ==
                    clang::TemplateArgument::ArgKind::TemplateExpansion) {
                    // pass
                }
                else if (template_argument.getAsType()
                             ->getAs<clang::FunctionProtoType>()) {
                    for (const auto &param_type :
                        template_argument.getAsType()
                            ->getAs<clang::FunctionProtoType>()
                            ->param_types()) {
                        result = find_relationships(param_type, relationships,
                            relationship_t::kDependency);
                    }
                }
                else if (template_argument_kind ==
                    clang::TemplateArgument::ArgKind::Type) {
                    result = find_relationships(template_argument.getAsType(),
                        relationships, relationship_hint);
                }
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
            if (type_element_id != c.id() &&
                (relationship_type != relationship_t::kNone)) {
                relationship r{relationship_t::kDependency, type_element_id};

                LOG_DBG(
                    "Adding function parameter relationship from {} to {}: {}",
                    c.full_name(), clanguml::common::model::to_string(r.type()),
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
    const class_member &field, const found_relationships_t &relationships,
    bool break_on_first_aggregation)
{
    auto [decorator_rtype, decorator_rmult] = field.get_relationship();

    for (const auto &[target, relationship_type] : relationships) {
        if (relationship_type != relationship_t::kNone) {
            relationship r{relationship_type, target};
            r.set_label(field.name());
            r.set_access(field.access());
            if (decorator_rtype != relationship_t::kNone) {
                r.set_type(decorator_rtype);
                auto mult = util::split(decorator_rmult, ":", false);
                if (mult.size() == 2) {
                    r.set_multiplicity_source(mult[0]);
                    r.set_multiplicity_destination(mult[1]);
                }
            }
            r.set_style(field.style_spec());

            LOG_DBG("Adding relationship from {} to {} with label {}",
                c.full_name(false), r.destination(),
                clanguml::common::model::to_string(r.type()), r.label());

            c.add_relationship(std::move(r));

            if (break_on_first_aggregation &&
                relationship_type == relationship_t::kAggregation)
                break;
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

        add_relationships(c, field, relationships);
    }

    c.add_member(std::move(field));
}

std::unique_ptr<class_>
translation_unit_visitor::process_template_specialization(
    clang::ClassTemplateSpecializationDecl *cls)
{
    auto c_ptr{std::make_unique<class_>(config_.using_namespace())};
    auto &template_instantiation = *c_ptr;

    // TODO: refactor to method get_qualified_name()
    auto qualified_name = cls->getQualifiedNameAsString();
    util::replace_all(qualified_name, "(anonymous namespace)", "");
    util::replace_all(qualified_name, "::::", "::");

    namespace_ ns{qualified_name};
    ns.pop_back();
    template_instantiation.set_name(cls->getNameAsString());
    template_instantiation.set_namespace(ns);
    template_instantiation.set_id(cls->getID());

    template_instantiation.is_struct(cls->isStruct());

    process_comment(*cls, template_instantiation);
    set_source_location(*cls, template_instantiation);

    if (template_instantiation.skip())
        return {};

    const auto template_args_count = cls->getTemplateArgs().size();
    for (auto arg_it = 0U; arg_it < template_args_count; arg_it++) {
        const auto arg = cls->getTemplateArgs().get(arg_it);
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
                        {&template_instantiation});

                argument.set_id(nested_template_instantiation->id());

                for (const auto &t : nested_template_instantiation->templates())
                    argument.add_template_param(t);

                // Check if this template should be simplified (e.g. system
                // template aliases such as 'std:basic_string<char>' should be
                // simply 'std::string')
                simplify_system_template(argument,
                    argument.to_string(config().using_namespace(), false));
            }
            else {
                auto type_name =
                    to_string(arg.getAsType(), cls->getASTContext());
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
                else
                    // Otherwise just set the name for the template argument to
                    // whatever clang says
                    argument.set_name(type_name);
            }

            LOG_DBG("Adding template instantiation argument {}",
                argument.to_string(config().using_namespace(), false));

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

    return c_ptr;
}

void translation_unit_visitor::
    process_unexposed_template_specialization_parameters(
        const std::string &type_name, template_parameter &tp, class_ &c)
{
    auto template_params = cx::util::parse_unexposed_template_params(
        type_name, [this](const std::string &t) {
            //            auto full_type = ctx.get_name_with_namespace(t);
            //            if (full_type.has_value())
            //                return full_type.value().to_string();
            return t;
        });

    found_relationships_t relationships;
    for (auto &param : template_params) {
        find_relationships_in_unexposed_template_params(param, relationships);
        tp.add_template_param(param);
    }

    for (auto &r : relationships) {
        c.add_relationship({std::get<1>(r), std::get<0>(r)});
    }

    //    const auto &primary_template_ref =
    //        static_cast<const cppast::cpp_class_template &>(
    //            tspec.value().primary_template().get(ctx.entity_index())[0].get())
    //            .class_();

    //    if (primary_template_ref.user_data()) {
    //        auto base_template_full_name =
    //            static_cast<const char *>(primary_template_ref.user_data());
    //        LOG_DBG("Primary template ref set to: {}",
    //        base_template_full_name);
    //        // Add template specialization/instantiation
    //        // relationship
    //        c.add_relationship(
    //            {relationship_t::kInstantiation, base_template_full_name});
    //    }
    //    else {
    //        LOG_DBG(
    //            "No user data for base template {}",
    //            primary_template_ref.name());
    //    }
}

bool translation_unit_visitor::find_relationships_in_unexposed_template_params(
    const template_parameter &ct, found_relationships_t &relationships)
{
    bool found{false};
    LOG_DBG("Finding relationships in user defined type: {}",
        ct.to_string(config().using_namespace(), false));

    //    auto type_with_namespace = ctx.get_name_with_namespace(ct.type());
    auto type_with_namespace =
        std::make_optional<common::model::namespace_>(ct.type());

    if (!type_with_namespace.has_value()) {
        // Couldn't find declaration of this type
        type_with_namespace = common::model::namespace_{ct.type()};
    }

    auto element_opt = diagram().get(type_with_namespace.value().to_string());
    if (element_opt) {
        relationships.emplace_back(
            element_opt.value().get().id(), relationship_t::kDependency);
        found = true;
    }

    for (const auto &nested_template_params : ct.template_params()) {
        found = find_relationships_in_unexposed_template_params(
                    nested_template_params, relationships) ||
            found;
    }
    return found;
}

std::unique_ptr<class_> translation_unit_visitor::build_template_instantiation(
    const clang::TemplateSpecializationType &template_type_decl,
    std::optional<clanguml::class_diagram::model::class_ *> parent)
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
    if (template_type_decl.isTypeAlias())
        template_type_ptr = template_type_decl.getAliasedType()
                                ->getAs<clang::TemplateSpecializationType>();

    auto &template_type = *template_type_ptr;

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
            const auto base_class_name = to_string(
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

    // TODO: Refactor handling of template arguments to a separate method
    auto arg_index = 0U;
    for (const auto &arg : template_type) {
        const auto argument_kind = arg.getKind();
        template_parameter argument;
        if (argument_kind == clang::TemplateArgument::ArgKind::Template) {
            argument.is_template_parameter(true);
            auto arg_name = arg.getAsTemplate()
                                .getAsTemplateDecl()
                                ->getQualifiedNameAsString();
            argument.set_type(arg_name);
        }
        else if (argument_kind == clang::TemplateArgument::ArgKind::Type) {
            argument.is_template_parameter(false);

            // If this is a nested template type - add nested templates as
            // template arguments
            if (arg.getAsType()->getAs<clang::FunctionType>()) {
                for (const auto &param_type :
                    arg.getAsType()
                        ->getAs<clang::FunctionProtoType>()
                        ->param_types()) {

                    const auto *nested_template_type =
                        param_type->getAs<clang::TemplateSpecializationType>();

                    if (nested_template_type) {
                        const auto nested_template_name =
                            nested_template_type->getTemplateName()
                                .getAsTemplateDecl()
                                ->getQualifiedNameAsString();

                        auto [tinst_ns, tinst_name] =
                            cx::util::split_ns(nested_template_name);

                        auto nested_template_instantiation =
                            build_template_instantiation(
                                *param_type->getAs<
                                    clang::TemplateSpecializationType>(),
                                diagram().should_include(
                                    full_template_specialization_name)
                                    ? std::make_optional(
                                          &template_instantiation)
                                    : parent);

                        if (nested_template_instantiation &&
                            diagram().should_include(
                                full_template_specialization_name)) {
                            if (diagram().should_include(
                                    tinst_ns, tinst_name)) {
                                template_instantiation.add_relationship(
                                    {relationship_t::kDependency,
                                        nested_template_instantiation->id()});
                            }
                            else {
                                if (parent.has_value())
                                    parent.value()->add_relationship(
                                        {relationship_t::kDependency,
                                            nested_template_instantiation
                                                ->id()});
                            }
                        }
                    }
                }
            }
            else if (arg.getAsType()
                         ->getAs<clang::TemplateSpecializationType>()) {
                const auto *nested_template_type =
                    arg.getAsType()->getAs<clang::TemplateSpecializationType>();

                const auto nested_template_name =
                    nested_template_type->getTemplateName()
                        .getAsTemplateDecl()
                        ->getQualifiedNameAsString();

                auto [tinst_ns, tinst_name] =
                    cx::util::split_ns(nested_template_name);

                argument.set_name(nested_template_name);

                auto nested_template_instantiation =
                    build_template_instantiation(
                        *arg.getAsType()
                             ->getAs<clang::TemplateSpecializationType>(),
                        diagram().should_include(
                            full_template_specialization_name)
                            ? std::make_optional(&template_instantiation)
                            : parent);

                argument.set_id(nested_template_instantiation->id());

                for (const auto &t : nested_template_instantiation->templates())
                    argument.add_template_param(t);

                // Check if this template should be simplified (e.g. system
                // template aliases such as 'std:basic_string<char>' should be
                // simply 'std::string')
                simplify_system_template(argument,
                    argument.to_string(config().using_namespace(), false));

                if (nested_template_instantiation &&
                    diagram().should_include(
                        nested_template_instantiation->full_name(false))) {
                    if (diagram().should_include(
                            full_template_specialization_name)) {
                        template_instantiation.add_relationship(
                            {relationship_t::kDependency,
                                nested_template_instantiation->id()});
                    }
                    else {
                        if (parent.has_value())
                            parent.value()->add_relationship(
                                {relationship_t::kDependency,
                                    nested_template_instantiation->id()});
                    }
                }

                auto nested_template_instantiation_full_name =
                    nested_template_instantiation->full_name(false);
                if (diagram().should_include(
                        nested_template_instantiation_full_name)) {
                    diagram().add_class(
                        std::move(nested_template_instantiation));
                }
            }
            else if (arg.getAsType()->getAs<clang::TemplateTypeParmType>()) {
                argument.is_template_parameter(true);
                argument.set_name(
                    to_string(arg.getAsType(), template_decl->getASTContext()));
            }
            else {
                // This is just a regular type
                argument.is_template_parameter(false);

                argument.set_name(
                    to_string(arg.getAsType(), template_decl->getASTContext()));

                if (arg.getAsType()->getAs<clang::RecordType>() &&
                    arg.getAsType()
                        ->getAs<clang::RecordType>()
                        ->getAsRecordDecl()) {
                    argument.set_id(arg.getAsType()
                                        ->getAs<clang::RecordType>()
                                        ->getAsRecordDecl()
                                        ->getID());

                    if (diagram().should_include(
                            full_template_specialization_name)) {
                        // Add dependency relationship to the parent template
                        template_instantiation.add_relationship(
                            {relationship_t::kDependency,
                                arg.getAsType()
                                    ->getAs<clang::RecordType>()
                                    ->getAsRecordDecl()
                                    ->getID()});
                    }
                }
                else if (arg.getAsType()->getAs<clang::EnumType>()) {
                    if (arg.getAsType()
                            ->getAs<clang::EnumType>()
                            ->getAsTagDecl()) {
                        template_instantiation.add_relationship(
                            {relationship_t::kDependency,
                                arg.getAsType()
                                    ->getAs<clang::EnumType>()
                                    ->getAsTagDecl()
                                    ->getID()});
                    }
                }
            }
        }
        else if (argument_kind == clang::TemplateArgument::ArgKind::Integral) {
            argument.is_template_parameter(false);
            argument.set_type(
                std::to_string(arg.getAsIntegral().getExtValue()));
        }
        else if (argument_kind ==
            clang::TemplateArgument::ArgKind::Expression) {
            argument.is_template_parameter(false);
            argument.set_type(get_source_text(
                arg.getAsExpr()->getSourceRange(), source_manager_));
        }
        else {
            LOG_ERROR("Unsupported argument type {}", arg.getKind());
        }

        // We can figure this only when we encounter variadic param in
        // the list of template params, from then this variable is true
        // and we can process following template parameters as belonging
        // to the variadic tuple
        auto variadic_params = false;

        // In case any of the template arguments are base classes, add
        // them as parents of the current template instantiation class
        if (template_base_params.size() > 0) {
            variadic_params = build_template_instantiation_add_base_classes(
                template_instantiation, template_base_params, arg_index,
                variadic_params, argument);
        }

        LOG_DBG("Adding template argument {} to template "
                "specialization/instantiation {}",
            argument.name(), template_instantiation.name());

        simplify_system_template(
            argument, argument.to_string(config().using_namespace(), false));
        template_instantiation.add_template(std::move(argument));

        arg_index++;
    }

    // First try to find the best match for this template in partially
    // specialized templates
    std::string destination{};
    std::string best_match_full_name{};
    auto full_template_name = template_instantiation.full_name(false);
    int best_match{};
    common::model::diagram_element::id_t best_match_id{};

    for (const auto c : diagram().classes()) {
        if (c.get() == template_instantiation)
            continue;

        auto c_full_name = c.get().full_name(false);
        auto match = c.get().calculate_template_specialization_match(
            template_instantiation, template_instantiation.name_and_ns());

        if (match > best_match) {
            best_match = match;
            best_match_full_name = c_full_name;
            best_match_id = c.get().id();
        }
    }

    if (!best_match_full_name.empty()) {
        destination = best_match_full_name;
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, best_match_id});
    }
    // If we can't find optimal match for parent template specialization, just
    // use whatever clang suggests
    else if (diagram().has_element(template_type.getTemplateName()
                                       .getAsTemplateDecl()
                                       ->getID())) {
        template_instantiation.add_relationship({relationship_t::kInstantiation,
            template_type.getTemplateName().getAsTemplateDecl()->getID()});
    }

    return template_instantiation_ptr;
}

bool translation_unit_visitor::build_template_instantiation_add_base_classes(
    class_ &tinst,
    std::deque<std::tuple<std::string, int, bool>> &template_base_params,
    int arg_index, bool variadic_params, const template_parameter &ct) const
{
    bool add_template_argument_as_base_class = false;

    auto [arg_name, index, is_variadic] = template_base_params.front();
    if (variadic_params)
        add_template_argument_as_base_class = true;
    else {
        variadic_params = is_variadic;
        if ((arg_index == index) || (is_variadic && arg_index >= index)) {
            add_template_argument_as_base_class = true;
            if (!is_variadic) {
                // Don't remove the remaining variadic parameter
                template_base_params.pop_front();
            }
        }
    }

    if (add_template_argument_as_base_class && ct.id()) {
        LOG_DBG("Adding template argument as base class '{}'",
            ct.to_string({}, false));

        class_parent cp;
        cp.set_access(access_t::kPublic);
        cp.set_name(ct.to_string({}, false));
        cp.set_id(ct.id().value());

        tinst.add_parent(std::move(cp));
    }

    return variadic_params;
}

void translation_unit_visitor::process_field(
    const clang::FieldDecl &field_declaration, class_ &c)
{
    // Default hint for relationship is aggregation
    auto relationship_hint = relationship_t::kAggregation;
    // If the first type of the template instantiation of this field type
    // has been added as aggregation relationship with class 'c', don't
    // add it's nested template types as aggregation
    [[maybe_unused]] bool template_instantiation_added_as_aggregation{false};
    // The actual field type
    auto field_type = field_declaration.getType();
    // String representation of the field type
    auto type_name = to_string(field_type, field_declaration.getASTContext());
    // The field name
    const auto field_name = field_declaration.getNameAsString();
    // If for any reason clang reports the type as empty string, make sure it
    // has some default name
    if (type_name.empty())
        type_name = "<<anonymous>>";

    class_member field{
        detail::access_specifier_to_access_t(field_declaration.getAccess()),
        field_name,
        to_string(field_type, field_declaration.getASTContext(), false)};

    // Parse the field comment
    process_comment(field_declaration, field);
    // Register the source location of the field declaration
    set_source_location(field_declaration, field);

    // If the comment contains a skip directive, just return
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

    if (type_name.find("std::shared_ptr") == 0)
        relationship_hint = relationship_t::kAssociation;
    if (type_name.find("std::weak_ptr") == 0)
        relationship_hint = relationship_t::kAssociation;

    const auto *template_field_type =
        field_type->getAs<clang::TemplateSpecializationType>();

    found_relationships_t relationships;

    // TODO: Refactor to an unalias_type() method
    if (template_field_type != nullptr)
        if (template_field_type->isTypeAlias())
            template_field_type =
                template_field_type->getAliasedType()
                    ->getAs<clang::TemplateSpecializationType>();

    bool field_type_is_template_template_parameter{false};
    if (template_field_type != nullptr) {
        // Skip types which are templatetemplate parameters of the parent
        // template
        for (const auto &class_template_param : c.templates()) {
            if (class_template_param.name() ==
                template_field_type->getTemplateName()
                        .getAsTemplateDecl()
                        ->getNameAsString() +
                    "<>") {
                field_type_is_template_template_parameter = true;
            }
        }
    }

    // Process the type which is template instantiation of some sort
    if (template_field_type != nullptr &&
        !field_type_is_template_template_parameter) {
        const auto template_field_decl_name =
            template_field_type->getTemplateName()
                .getAsTemplateDecl()
                ->getQualifiedNameAsString();

        // Build the template instantiation for the field type
        auto template_specialization_ptr = build_template_instantiation(
            *field_type->getAs<clang::TemplateSpecializationType>(), {&c});

        if (!field.skip_relationship() && template_specialization_ptr) {
            const auto &template_specialization = *template_specialization_ptr;

            // Check if this template instantiation should be added to the
            // current diagram. Even if the top level template type for
            // this instantiation should not be part of the diagram, e.g. it's
            // a std::vector<>, it's nested types might be added
            bool add_template_instantiation_to_diargam{false};
            if (diagram().should_include(
                    template_specialization.full_name(false))) {

                found_relationships_t::value_type r{
                    template_specialization.id(), relationship_hint};

                add_template_instantiation_to_diargam = true;

                // If the template instantiation for the build type has been
                // added as aggregation, skip its nested templates
                template_instantiation_added_as_aggregation =
                    relationship_hint == relationship_t::kAggregation;
                relationships.emplace_back(std::move(r));
            }

            // Try to find relationships to types nested in the template
            // instantiation
            found_relationships_t nested_relationships;
            if (!template_instantiation_added_as_aggregation) {
                for (const auto &template_argument :
                    template_specialization.templates()) {

                    LOG_DBG("Looking for nested relationships from {}:{} in "
                            "template {}",
                        c.full_name(false), field_name,
                        template_argument.to_string(
                            config().using_namespace(), false));

                    template_instantiation_added_as_aggregation =
                        template_argument.find_nested_relationships(
                            nested_relationships, relationship_hint,
                            [&d = diagram()](const std::string &full_name) {
                                if (full_name.empty())
                                    return false;
                                auto [ns, name] = cx::util::split_ns(full_name);
                                return d.should_include(ns, name);
                            });
                }

                // Add any relationships to the class 'c' to the diagram, unless
                // the top level type has been added as aggregation
                add_relationships(c, field, nested_relationships,
                    /* break on first aggregation */ false);
            }

            // Add the template instantiation object to the diagram if it
            // matches the include pattern
            if (add_template_instantiation_to_diargam)
                diagram().add_class(std::move(template_specialization_ptr));
        }
    }

    if (!field.skip_relationship()) {
        // Find relationship for the type if the type has not been added
        // as aggregation
        if (!template_instantiation_added_as_aggregation)
            find_relationships(field_type, relationships, relationship_hint);

        add_relationships(c, field, relationships);
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

void translation_unit_visitor::add_incomplete_forward_declarations()
{
    for (auto &[id, c] : forward_declarations_) {
        if (diagram().should_include(c->full_name(false))) {
            diagram().add_class(std::move(c));
        }
    }
    forward_declarations_.clear();
}

void translation_unit_visitor::finalize()
{
    add_incomplete_forward_declarations();
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
