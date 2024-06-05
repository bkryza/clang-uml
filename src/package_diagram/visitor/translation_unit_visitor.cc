/**
 * @file src/package_diagram/visitor/translation_unit_visitor.cc
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

#include "clang/Basic/Module.h"

#include <spdlog/spdlog.h>

#include <deque>

namespace clanguml::package_diagram::visitor {

using clanguml::common::model::namespace_;
using clanguml::common::model::package;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::package_diagram::model::diagram &diagram,
    const clanguml::config::package_diagram &config)
    : visitor_specialization_t{sm, diagram, config}
{
}

bool translation_unit_visitor::VisitNamespaceDecl(clang::NamespaceDecl *ns)
{
    assert(ns != nullptr);

    if (config().package_type() != config::package_type_t::kNamespace)
        return true;

    if (ns->isAnonymousNamespace() || ns->isInline())
        return true;

    auto qualified_name = common::get_qualified_name(*ns);

    if (!diagram().should_include(namespace_{qualified_name}))
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
    set_source_location(*ns, *p);

    assert(p->id().value() > 0);

    if (diagram().should_include(*p) && !diagram().get(p->id())) {
        process_comment(*ns, *p);

        p->set_style(p->style_spec());

        for (const auto *attr : ns->attrs()) {
            if (attr->getKind() == clang::attr::Kind::Deprecated) {
                p->set_deprecated(true);
                break;
            }
        }

        if (!p->skip()) {
            LOG_DBG("Adding package {}", p->full_name(false));

            diagram().add(p->path(), std::move(p));
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

bool translation_unit_visitor::VisitRecordDecl(clang::RecordDecl *decl)
{
    assert(decl != nullptr);

    // Skip system headers
    if (source_manager().isInSystemHeader(decl->getSourceRange().getBegin()))
        return true;

    found_relationships_t relationships;

    if (decl->isCompleteDefinition()) {
        process_record_children(*decl, relationships);
        add_relationships(decl, relationships);
    }

    return true;
}

bool translation_unit_visitor::VisitEnumDecl(clang::EnumDecl *decl)
{
    assert(decl != nullptr);

    // Skip system headers
    if (source_manager().isInSystemHeader(decl->getSourceRange().getBegin()))
        return true;

    found_relationships_t relationships;

    if (decl->isCompleteDefinition()) {
        add_relationships(decl, relationships);
    }

    return true;
}

bool translation_unit_visitor::VisitClassTemplateDecl(
    clang::ClassTemplateDecl *decl)
{
    assert(decl != nullptr);

    // Skip system headers
    if (source_manager().isInSystemHeader(decl->getSourceRange().getBegin()))
        return true;

    found_relationships_t relationships;

    util::if_not_null(decl->getTemplatedDecl(),
        [this, &relationships, decl](const auto *template_decl) {
            if (template_decl->isCompleteDefinition()) {
                process_class_declaration(*template_decl, relationships);
                add_relationships(decl, relationships);
            }
        });

    return true;
}

void translation_unit_visitor::add_relationships(
    clang::Decl *cls, found_relationships_t &relationships)
{
    // If this diagram has directory packages, first make sure that the
    // package for current directory is already in the model
    if (config().package_type() == config::package_type_t::kDirectory) {
        auto file = source_manager().getFilename(cls->getLocation()).str();

        if (file.empty())
            return;

        auto relative_file = config().make_path_relative(file);
        relative_file.make_preferred();

        common::model::path parent_path{
            relative_file.string(), common::model::path_type::kFilesystem};
        parent_path.pop_back();
        auto pkg_name = parent_path.name();
        parent_path.pop_back();

        auto pkg = std::make_unique<common::model::package>(
            config().using_namespace());

        pkg->set_name(pkg_name);
        pkg->set_id(get_package_id(cls));
        set_source_location(*cls, *pkg);

        if (diagram().should_include(*pkg))
            diagram().add(parent_path, std::move(pkg));
    }
    else if (config().package_type() == config::package_type_t::kModule) {
        const auto *module = cls->getOwningModule();

        if (module == nullptr) {
            return;
        }

        std::string module_path_str = module->Name;
#if LLVM_VERSION_MAJOR < 15
        if (module->Kind == clang::Module::ModuleKind::PrivateModuleFragment) {
#else
        if (module->isPrivateModule()) {
#endif
            module_path_str = module->getTopLevelModule()->Name;
        }

        common::model::path module_path{
            module_path_str, common::model::path_type::kModule};
        module_path.pop_back();

        auto relative_module =
            config().make_module_relative(std::optional{module_path_str});

        common::model::path parent_path{
            relative_module, common::model::path_type::kModule};
        auto pkg_name = parent_path.name();
        parent_path.pop_back();

        auto pkg = std::make_unique<common::model::package>(
            config().using_module(), common::model::path_type::kModule);

        pkg->set_name(pkg_name);
        pkg->set_id(get_package_id(cls));
        // This is for diagram filters
        pkg->set_module(module_path.to_string());
        // This is for rendering nested package structure
        pkg->set_namespace(module_path);
        set_source_location(*cls, *pkg);

        if (diagram().should_include(*pkg))
            diagram().add(parent_path, std::move(pkg));
    }

    auto current_package_id = get_package_id(cls);

    if (current_package_id.value() == 0)
        // These are relationships to a global namespace, and we don't care
        // about those
        return;

    auto current_package = diagram().get(current_package_id);

    if (current_package) {
        std::vector<eid_t> parent_ids =
            get_parent_package_ids(current_package_id);

        for (const auto &dependency : relationships) {
            const auto destination_id = std::get<0>(dependency);

            // Skip dependency relationships to parent packages
            if (util::contains(parent_ids, destination_id))
                continue;

            // Skip dependency relationship to child packages
            if (util::contains(
                    get_parent_package_ids(destination_id), current_package_id))
                continue;

            relationship r{relationship_t::kDependency, destination_id,
                common::model::access_t::kNone};
            if (destination_id != current_package_id)
                current_package.value().add_relationship(std::move(r));
        }
    }
}

eid_t translation_unit_visitor::get_package_id(const clang::Decl *cls)
{
    if (config().package_type() == config::package_type_t::kNamespace) {
        const auto *namespace_context =
            cls->getDeclContext()->getEnclosingNamespaceContext();
        if (namespace_context != nullptr && namespace_context->isNamespace()) {
            return common::to_id(
                *llvm::cast<clang::NamespaceDecl>(namespace_context));
        }

        return {};
    }

    if (config().package_type() == config::package_type_t::kModule) {
        const auto *module = cls->getOwningModule();
        if (module != nullptr) {
            std::string module_path = module->Name;
#if LLVM_VERSION_MAJOR < 15
            if (module->Kind ==
                clang::Module::ModuleKind::PrivateModuleFragment) {
#else
            if (module->isPrivateModule()) {
#endif
                module_path = module->getTopLevelModule()->Name;
            }
            return common::to_id(module_path);
        }

        return {};
    }

    auto file =
        source_manager().getFilename(cls->getSourceRange().getBegin()).str();
    auto relative_file = config().make_path_relative(file);
    relative_file.make_preferred();
    common::model::path parent_path{
        relative_file.string(), common::model::path_type::kFilesystem};
    parent_path.pop_back();

    return common::to_id(parent_path.to_string());
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
                clang::dyn_cast_or_null<clang::VarDecl>(decl)};
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

void translation_unit_visitor::process_record_children(
    const clang::RecordDecl &cls, found_relationships_t &relationships)
{
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
                clang::dyn_cast_or_null<clang::VarDecl>(decl)};
            if ((variable_declaration != nullptr) &&
                variable_declaration->isStaticDataMember()) {
                process_static_field(*variable_declaration, relationships);
            }
        }
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
            if (const auto *enum_decl = enum_type->getDecl();
                enum_decl != nullptr)
                relationships.emplace_back(
                    get_package_id(enum_decl), relationship_hint);
        }
    }
    else if (const auto *template_specialization_type =
                 type->getAs<clang::TemplateSpecializationType>()) {
        if (template_specialization_type != nullptr) {
            // Add dependency to template declaration
            relationships.emplace_back(
                get_package_id(template_specialization_type->getTemplateName()
                                   .getAsTemplateDecl()),
                relationship_hint);

            // Add dependencies to template arguments
            for (const auto &template_argument :
                template_specialization_type->template_arguments()) {
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
        if (const auto *cxxrecord_decl = type->getAsCXXRecordDecl();
            cxxrecord_decl != nullptr) {
            if (config().package_type() == config::package_type_t::kNamespace) {
                const auto *namespace_context =
                    cxxrecord_decl->getEnclosingNamespaceContext();
                if (namespace_context != nullptr &&
                    namespace_context->isNamespace()) {
                    const auto *namespace_declaration =
                        clang::cast<clang::NamespaceDecl>(namespace_context);

                    if (namespace_declaration != nullptr &&
                        diagram().should_include(
                            namespace_{common::get_qualified_name(
                                *namespace_declaration)})) {
                        const auto target_id = get_package_id(cxxrecord_decl);
                        relationships.emplace_back(
                            target_id, relationship_hint);
                        result = true;
                    }
                }
            }
            else if (config().package_type() ==
                config::package_type_t::kModule) {
                const auto *module = cxxrecord_decl->getOwningModule();
                if (module != nullptr) {
                    const auto target_id = get_package_id(cxxrecord_decl);
                    relationships.emplace_back(target_id, relationship_hint);
                    result = true;
                }
            }
            else {
                if (diagram().should_include(
                        namespace_{common::get_qualified_name(
                            *type->getAsCXXRecordDecl())})) {
                    const auto target_id =
                        get_package_id(type->getAsCXXRecordDecl());
                    relationships.emplace_back(target_id, relationship_hint);
                    result = true;
                }
            }
        }
        else if (const auto *record_decl = type->getAsRecordDecl();
                 record_decl != nullptr) {
            // This is only possible for plain C translation unit, so we
            // don't need to consider namespaces or modules here
            if (config().package_type() == config::package_type_t::kDirectory) {
                if (diagram().should_include(
                        namespace_{common::get_qualified_name(*record_decl)})) {
                    const auto target_id = get_package_id(record_decl);
                    relationships.emplace_back(target_id, relationship_hint);
                    result = true;
                }
            }
        }
    }

    return result;
}

void translation_unit_visitor::finalize() { }

std::vector<eid_t> translation_unit_visitor::get_parent_package_ids(eid_t id)
{
    std::vector<eid_t> parent_ids;
    std::optional<eid_t> parent_id = id;

    while (parent_id.has_value()) {
        const auto pid = parent_id.value(); // NOLINT
        parent_ids.push_back(pid);
        auto parent = this->diagram().get(pid);
        if (parent)
            parent_id = parent.value().parent_element_id();
        else
            break;
    }

    return parent_ids;
}

} // namespace clanguml::package_diagram::visitor
