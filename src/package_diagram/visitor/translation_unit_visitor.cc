/**
 * src/package_diagram/visitor/translation_unit_visitor.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
#include "cx/util.h"

#include <spdlog/spdlog.h>

#include <deque>

namespace clanguml::package_diagram::visitor {

using clanguml::common::model::access_t;
using clanguml::common::model::namespace_;
using clanguml::common::model::package;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;
using clanguml::package_diagram::model::diagram;

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::package_diagram::model::diagram &diagram,
    const clanguml::config::package_diagram &config)
    : common::visitor::translation_unit_visitor{sm, config}
    , diagram_{diagram}
    , config_{config}
{
}

bool translation_unit_visitor::VisitNamespaceDecl(clang::NamespaceDecl *ns)
{
    assert(ns != nullptr);

    if (ns->isAnonymousNamespace() || ns->isInline())
        return true;

    auto qualified_name = common::get_qualified_name(*ns);

    if (!diagram().should_include(qualified_name))
        return true;

    LOG_DBG("Visiting namespace declaration: {}", qualified_name);

    auto package_path = namespace_{qualified_name};
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

    assert(p->id() > 0);

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

bool translation_unit_visitor::VisitFunctionDecl(
    clang::FunctionDecl *function_declaration)
{
    assert(function_declaration != nullptr);

    // Skip system headers
    if (source_manager().isInSystemHeader(
            function_declaration->getSourceRange().getBegin()))
        return true;

    found_relationships_t relationships;

    find_relationships(function_declaration->getReturnType(), relationships);

    for (const auto *param : function_declaration->parameters()) {
        if (param != nullptr)
            find_relationships(param->getType(), relationships);
    }

    add_relationships(function_declaration, relationships);

    return true;
}

bool translation_unit_visitor::VisitCXXRecordDecl(clang::CXXRecordDecl *cls)
{
    assert(cls != nullptr);

    // Skip system headers
    if (source_manager().isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    // Templated records are handled by VisitClassTemplateDecl()
    if (cls->isTemplated() || cls->isTemplateDecl() ||
        (clang::dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(cls) !=
            nullptr))
        return true;

    found_relationships_t relationships;

    if (cls->isCompleteDefinition()) {
        process_class_declaration(*cls, relationships);
        add_relationships(cls, relationships);
    }

    return true;
}
void translation_unit_visitor::add_relationships(
    clang::DeclContext *cls, found_relationships_t &relationships)
{
    int64_t current_package_id{0};

    const auto *namespace_context = cls->getEnclosingNamespaceContext();
    if (namespace_context != nullptr && namespace_context->isNamespace()) {
        current_package_id =
            common::to_id(*llvm::cast<clang::NamespaceDecl>(namespace_context));
    }

    if (current_package_id == 0)
        // These are relationships to a global namespace, and we don't care
        // about those
        return;

    assert(current_package_id != 0);

    auto current_package = diagram().get(current_package_id);

    if (current_package) {
        for (const auto &dependency : relationships) {
            const auto destination_id = std::get<0>(dependency);
            relationship r{relationship_t::kDependency, destination_id};
            if (destination_id != current_package_id)
                current_package.value().add_relationship(std::move(r));
        }
    }
}

void translation_unit_visitor::process_class_declaration(
    const clang::CXXRecordDecl &cls, found_relationships_t &relationships)
{
    // Look for dependency relationships in class children (fields, methods)
    process_class_children(cls, relationships);

    // Look for dependency relationships in class bases
    process_class_bases(cls, relationships);
}

void translation_unit_visitor::process_class_children(
    const clang::CXXRecordDecl &cls, found_relationships_t &relationships)
{
    // Iterate over class methods (both regular and static)
    for (const auto *method : cls.methods()) {
        if (method != nullptr) {
            process_method(*method, relationships);
        }
    }

    if (const auto *decl_context =
            clang::dyn_cast_or_null<clang::DeclContext>(&cls);
        decl_context != nullptr) {
        // Iterate over class template methods
        for (auto const *decl_iterator : decl_context->decls()) {
            auto const *method_template =
                llvm::dyn_cast_or_null<clang::FunctionTemplateDecl>(
                    decl_iterator);
            if (method_template == nullptr)
                continue;

            process_template_method(*method_template, relationships);
        }
    }

    // Iterate over regular class fields
    for (const auto *field : cls.fields()) {
        if (field != nullptr)
            process_field(*field, relationships);
    }

    // Static fields have to be processed by iterating over variable
    // declarations
    for (const auto *decl : cls.decls()) {
        if (decl->getKind() == clang::Decl::Var) {
            const clang::VarDecl *variable_declaration{
                dynamic_cast<const clang::VarDecl *>(decl)};
            if ((variable_declaration != nullptr) &&
                variable_declaration->isStaticDataMember()) {
                process_static_field(*variable_declaration, relationships);
            }
        }
    }

    if (cls.isCompleteDefinition())
        for (const auto *friend_declaration : cls.friends()) {
            if (friend_declaration != nullptr)
                process_friend(*friend_declaration, relationships);
        }
}

void translation_unit_visitor::process_class_bases(
    const clang::CXXRecordDecl &cls, found_relationships_t &relationships)
{
    for (const auto &base : cls.bases()) {
        find_relationships(base.getType(), relationships);
    }
}

void translation_unit_visitor::process_method(
    const clang::CXXMethodDecl &method, found_relationships_t &relationships)
{
    find_relationships(method.getReturnType(), relationships);

    for (const auto *param : method.parameters()) {
        if (param != nullptr)
            find_relationships(param->getType(), relationships);
    }
}

void translation_unit_visitor::process_template_method(
    const clang::FunctionTemplateDecl &method,
    found_relationships_t &relationships)
{
    // TODO: For now skip implicitly default methods
    //       in the future, add config option to choose
    if (method.getTemplatedDecl()->isDefaulted() &&
        !method.getTemplatedDecl()->isExplicitlyDefaulted())
        return;

    find_relationships(
        method.getTemplatedDecl()->getReturnType(), relationships);

    for (const auto *param : method.getTemplatedDecl()->parameters()) {
        if (param != nullptr) {
            find_relationships(param->getType(), relationships);
        }
    }
}

void translation_unit_visitor::process_field(
    const clang::FieldDecl &field_declaration,
    found_relationships_t &relationships)
{
    find_relationships(field_declaration.getType(), relationships,
        relationship_t::kDependency);
}

void translation_unit_visitor::process_static_field(
    const clang::VarDecl &field_declaration,
    found_relationships_t &relationships)
{
    find_relationships(field_declaration.getType(), relationships,
        relationship_t::kDependency);
}

void translation_unit_visitor::process_friend(
    const clang::FriendDecl &friend_declaration,
    found_relationships_t &relationships)
{
    if (const auto *friend_type_declaration =
            friend_declaration.getFriendDecl()) {
        if (friend_type_declaration->isTemplateDecl()) {
            // TODO
        }
    }
    else if (const auto *friend_type = friend_declaration.getFriendType()) {
        find_relationships(friend_type->getType(), relationships);
    }
}

bool translation_unit_visitor::find_relationships(const clang::QualType &type,
    found_relationships_t &relationships, relationship_t relationship_hint)
{
    bool result{false};

    if (type->isVoidType() || type->isVoidPointerType()) {
        // pass
    }
    else if (type->isPointerType()) {
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
        if (const auto *enum_type = type->getAs<clang::EnumType>();
            enum_type != nullptr) {
            relationships.emplace_back(
                common::to_id(*enum_type), relationship_hint);
        }
    }
    else if (const auto *template_specialization_type =
                 type->getAs<clang::TemplateSpecializationType>()) {
        if (template_specialization_type != nullptr) {
            for (const auto &template_argument :
                *template_specialization_type) {
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
                else if (const auto *function_type =
                             template_argument.getAsType()
                                 ->getAs<clang::FunctionProtoType>();
                         function_type != nullptr) {
                    for (const auto &param_type :
                        function_type->param_types()) {
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
    }
    else if (type->isRecordType()) {
        const auto *namespace_context =
            type->getAsCXXRecordDecl()->getEnclosingNamespaceContext();
        if (namespace_context != nullptr && namespace_context->isNamespace()) {
            const auto *namespace_declaration =
                clang::cast<clang::NamespaceDecl>(namespace_context);

            if (namespace_declaration != nullptr &&
                diagram().should_include(
                    common::get_qualified_name(*namespace_declaration))) {
                const auto target_id = common::to_id(
                    *clang::cast<clang::NamespaceDecl>(namespace_context));
                relationships.emplace_back(target_id, relationship_hint);
                result = true;
            }
        }
    }

    return result;
}

} // namespace clanguml::package_diagram::visitor
