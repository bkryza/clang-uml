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
using clanguml::common::model::access_t;
using clanguml::common::model::namespace_;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::class_diagram::model::diagram &diagram,
    const clanguml::config::class_diagram &config)
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

    LOG_DBG("= Visiting namespace declaration {} at {}",
        ns->getQualifiedNameAsString(),
        ns->getLocation().printToString(source_manager()));

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
    set_ast_local_id(ns->getID(), p->id());

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

    if (!diagram().should_include(enm->getQualifiedNameAsString()))
        return true;

    LOG_DBG("= Visiting enum declaration {} at {}",
        enm->getQualifiedNameAsString(),
        enm->getLocation().printToString(source_manager()));

    auto e_ptr = std::make_unique<enum_>(config_.using_namespace());
    auto &e = *e_ptr;

    std::string qualified_name = common::get_qualified_name(*enm);

    auto ns{common::get_tag_namespace(*enm)};

    const auto *parent = enm->getParent();

    std::optional<common::model::diagram_element::id_t> id_opt;

    if (parent != nullptr) {
        const auto *parent_record_decl =
            clang::dyn_cast<clang::RecordDecl>(parent);

        if (parent_record_decl != nullptr) {
            int64_t local_id = parent_record_decl->getID();

            // First check if the parent has been added to the diagram as
            // regular class
            id_opt = get_ast_local_id(local_id);

            // If not, check if the parent template declaration is in the model
            if (!id_opt) {
                if (parent_record_decl->getDescribedTemplate() != nullptr) {
                    local_id =
                        parent_record_decl->getDescribedTemplate()->getID();
                    id_opt = get_ast_local_id(local_id);
                }
            }
        }
    }

    if (id_opt) {
        auto parent_class = diagram_.get_class(*id_opt);

        assert(parent_class);

        e.set_namespace(ns);
        e.set_name(parent_class.value().name() + "##" + enm->getNameAsString());
        e.set_id(common::to_id(e.full_name(false)));
        e.add_relationship({relationship_t::kContainment, *id_opt});
        e.nested(true);
    }
    else {
        e.set_name(common::get_tag_name(*enm));
        e.set_namespace(ns);
        e.set_id(common::to_id(e.full_name(false)));
    }

    set_ast_local_id(enm->getID(), e.id());

    process_comment(*enm, e);
    set_source_location(*enm, e);

    if (e.skip())
        return true;

    e.set_style(e.style_spec());

    for (const auto &ev : enm->enumerators()) {
        e.constants().push_back(ev->getNameAsString());
    }

    auto namespace_declaration = common::get_enclosing_namespace(enm);
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
    if (source_manager().isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    if (!diagram().should_include(cls->getQualifiedNameAsString()))
        return true;

    LOG_DBG("= Visiting template specialization declaration {} at {}",
        cls->getQualifiedNameAsString(),
        cls->getLocation().printToString(source_manager()));

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass() != nullptr)
        return true;

    auto template_specialization_ptr = process_template_specialization(cls);

    if (!template_specialization_ptr)
        return true;

    auto &template_specialization = *template_specialization_ptr;

    process_template_specialization_children(cls, template_specialization);

    // Process template specialization bases
    process_class_bases(cls, template_specialization);

    if (get_ast_local_id(cls->getSpecializedTemplate()->getID()).has_value())
        template_specialization.add_relationship(
            {relationship_t::kInstantiation,
                get_ast_local_id(cls->getSpecializedTemplate()->getID())
                    .value()});

    if (diagram_.should_include(template_specialization)) {
        const auto name = template_specialization.full_name(false);
        const auto id = template_specialization.id();

        LOG_DBG("Adding class template specialization {} with id {}", name, id);

        diagram_.add_class(std::move(template_specialization_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitTypeAliasTemplateDecl(
    clang::TypeAliasTemplateDecl *cls)
{
    if (source_manager().isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    if (!diagram().should_include(cls->getQualifiedNameAsString()))
        return true;

    LOG_DBG("= Visiting template type alias declaration {} at {}",
        cls->getQualifiedNameAsString(),
        cls->getLocation().printToString(source_manager()));

    const auto *template_type_specialization_ptr =
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
        const auto name = template_specialization_ptr->full_name();
        const auto id = template_specialization_ptr->id();

        LOG_DBG("Adding class {} with id {}", name, id);

        diagram_.add_class(std::move(template_specialization_ptr));
    }

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
    process_class_declaration(*cls->getTemplatedDecl(), *c_ptr);
    forward_declarations_.erase(id);

    if (diagram_.should_include(*c_ptr)) {
        const auto name = c_ptr->full_name();
        LOG_DBG("Adding class template {} with id {}", name, id);

        diagram_.add_class(std::move(c_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitCXXRecordDecl(clang::CXXRecordDecl *cls)
{
    // Skip system headers
    if (source_manager().isInSystemHeader(cls->getSourceRange().getBegin()))
        return true;

    if (!diagram().should_include(cls->getQualifiedNameAsString()))
        return true;

    LOG_DBG("= Visiting class declaration {} at {}",
        cls->getQualifiedNameAsString(),
        cls->getLocation().printToString(source_manager()));

    LOG_DBG(
        "== getQualifiedNameAsString() = {}", cls->getQualifiedNameAsString());
    LOG_DBG("== getID() = {}", cls->getID());
    LOG_DBG("== isTemplateDecl() = {}", cls->isTemplateDecl());
    LOG_DBG("== isTemplated() = {}", cls->isTemplated());
    LOG_DBG("== getParent()->isRecord()() = {}", cls->getParent()->isRecord());
    if (cls->getParent()->isRecord()) {
        LOG_DBG("== getParent()->getQualifiedNameAsString() = {}",
            clang::dyn_cast<clang::RecordDecl>(cls->getParent())
                ->getQualifiedNameAsString());
    }

    if (cls->isTemplated() && (cls->getDescribedTemplate() != nullptr)) {
        // If the described templated of this class is already in the model
        // skip it:
        if (get_ast_local_id(cls->getDescribedTemplate()->getID()))
            return true;
    }

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass() != nullptr)
        return true;

    auto c_ptr = create_class_declaration(cls);

    if (!c_ptr)
        return true;

    const auto cls_id = c_ptr->id();

    set_ast_local_id(cls->getID(), cls_id);

    auto &class_model = diagram().get_class(cls_id).has_value()
        ? *diagram().get_class(cls_id).get()
        : *c_ptr;

    if (cls->isCompleteDefinition() && !class_model.complete())
        process_class_declaration(*cls, class_model);

    auto id = class_model.id();
    if (!cls->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(c_ptr));
        return true;
    }
    forward_declarations_.erase(id);

    if (diagram_.should_include(class_model)) {
        LOG_DBG("Adding class {} with id {}", class_model.full_name(false),
            class_model.id());

        diagram_.add_class(std::move(c_ptr));
    }
    else {
        LOG_DBG("Skipping class {} with id {}", class_model.full_name(),
            class_model.id());
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
    auto qualified_name =
        cls->getQualifiedNameAsString(); //  common::get_qualified_name(*cls);

    if (!diagram().should_include(qualified_name))
        return {};

    auto ns = common::get_tag_namespace(*cls);

    const auto *parent = cls->getParent();

    std::optional<common::model::diagram_element::id_t> id_opt;

    if (parent != nullptr) {
        const auto *parent_record_decl =
            clang::dyn_cast<clang::RecordDecl>(parent);

        if (parent_record_decl != nullptr) {
            int64_t local_id = parent_record_decl->getID();

            // First check if the parent has been added to the diagram as
            // regular class
            id_opt = get_ast_local_id(local_id);

            // If not, check if the parent template declaration is in the model
            if (!id_opt) {
                if (parent_record_decl->getDescribedTemplate() != nullptr) {
                    local_id =
                        parent_record_decl->getDescribedTemplate()->getID();
                    id_opt = get_ast_local_id(local_id);
                }
            }
        }
    }

    if (id_opt) {
        // Here we have 2 options, either:
        //  - the parent is a regular C++ class/struct
        //  - the parent is a class template declaration/specialization
        auto parent_class = diagram_.get_class(*id_opt);

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

        if (!cls->getNameAsString().empty()) {
            // Don't add anonymous structs as contained in the class
            // as they are already added as aggregations
            c.add_relationship({relationship_t::kContainment, *id_opt});
        }

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

void translation_unit_visitor::process_class_declaration(
    const clang::CXXRecordDecl &cls, class_ &c)
{
    // Process class child entities
    process_class_children(&cls, c);

    // Process class bases
    process_class_bases(&cls, c);

    c.complete(true);
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
        if (clang::dyn_cast_or_null<clang::TemplateTypeParmDecl>(parameter) !=
            nullptr) {
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
                     parameter) != nullptr) {
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
                     parameter) != nullptr) {
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

void translation_unit_visitor::process_template_record_containment(
    const clang::TagDecl &record,
    clanguml::common::model::element &element) const
{
    assert(record.getParent()->isRecord());

    const auto *parent = record.getParent(); //->getOuterLexicalRecordContext();

    if (parent != nullptr) {
        if (const auto *record_decl =
                clang::dyn_cast<clang::RecordDecl>(parent);
            record_decl != nullptr) {
            if (const auto *described_template =
                    record_decl->getDescribedTemplate();
                described_template != nullptr) {
                auto id_opt = get_ast_local_id(described_template->getID());

                if (id_opt) {
                    element.add_relationship(
                        {relationship_t::kContainment, *id_opt});
                }
            }
        }
    }
}

void translation_unit_visitor::process_record_containment(
    const clang::TagDecl &record,
    clanguml::common::model::element &element) const
{
    assert(record.getParent()->isRecord());

    const auto *parent = record.getParent()->getOuterLexicalRecordContext();
    auto parent_name = static_cast<const clang::RecordDecl *>(parent)
                           ->getQualifiedNameAsString();

    auto namespace_declaration = common::get_enclosing_namespace(parent);
    if (namespace_declaration.has_value()) {
        element.set_namespace(namespace_declaration.value());
    }

    if (const auto *record_decl =
            clang::dyn_cast<clang::RecordDecl>(record.getParent());
        record_decl != nullptr) {
        element.add_relationship(
            {relationship_t::kContainment, common::to_id(*record_decl)});
    }
}

void translation_unit_visitor::process_class_bases(
    const clang::CXXRecordDecl *cls, class_ &c)
{
    for (const auto &base : cls->bases()) {
        class_parent cp;
        auto name_and_ns = common::model::namespace_{
            common::to_string(base.getType(), cls->getASTContext())};

        cp.set_name(name_and_ns.to_string());

        if (const auto *record_type =
                base.getType()->getAs<clang::RecordType>();
            record_type != nullptr) {
            cp.set_id(common::to_id(*record_type->getDecl()));
        }
        else if (const auto *tsp =
                     base.getType()->getAs<clang::TemplateSpecializationType>();
                 tsp != nullptr) {
            auto template_specialization_ptr =
                build_template_instantiation(*tsp, {});
            if (template_specialization_ptr) {
                cp.set_id(template_specialization_ptr->id());
            }
        }
        else
            // This could be a template parameter - we don't want it here
            continue;

        cp.is_virtual(base.isVirtual());

        cp.set_access(
            common::access_specifier_to_access_t(base.getAccessSpecifier()));

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
    if (const auto *cls_decl_context =
            clang::dyn_cast_or_null<clang::DeclContext>(cls);
        cls_decl_context != nullptr) {
        for (auto const *decl_iterator :
            clang::dyn_cast_or_null<clang::DeclContext>(cls)->decls()) {
            auto const *method_template =
                llvm::dyn_cast_or_null<clang::FunctionTemplateDecl>(
                    decl_iterator);
            if (method_template == nullptr)
                continue;

            process_template_method(*method_template, c);
        }
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
            if ((variable_declaration != nullptr) &&
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
                    class_member m{common::access_specifier_to_access_t(
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
    if (const auto *cls_decl_context =
            clang::dyn_cast_or_null<clang::DeclContext>(cls);
        cls_decl_context != nullptr) {
        for (auto const *decl_iterator : cls_decl_context->decls()) {
            auto const *method_template =
                llvm::dyn_cast_or_null<clang::FunctionTemplateDecl>(
                    decl_iterator);
            if (method_template == nullptr)
                continue;

            process_template_method(*method_template, c);
        }
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
            if ((variable_declaration != nullptr) &&
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
                    class_member m{common::access_specifier_to_access_t(
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
        else if (friend_type->getAs<clang::RecordType>() != nullptr) {
            const auto friend_type_name =
                friend_type->getAsRecordDecl()->getQualifiedNameAsString();
            if (diagram().should_include(friend_type_name)) {
                relationship r{relationship_t::kFriendship,
                    common::to_id(*friend_type->getAsRecordDecl()),
                    common::access_specifier_to_access_t(f.getAccess()),
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

    class_method method{common::access_specifier_to_access_t(mf.getAccess()),
        util::trim(mf.getNameAsString()),
        common::to_string(mf.getReturnType(), mf.getASTContext())};

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

    class_method method{common::access_specifier_to_access_t(mf.getAccess()),
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
        if (const auto *enum_type = type->getAs<clang::EnumType>();
            enum_type != nullptr) {
            relationships.emplace_back(
                common::to_id(*enum_type->getDecl()), relationship_hint);
        }
    }
    else if (type->isRecordType()) {
        const auto *type_instantiation_decl =
            type->getAs<clang::TemplateSpecializationType>();

        //        if (type_instantiation_decl != nullptr) {
        //            if (type_instantiation_decl->isTypeAlias())
        //                type_instantiation_decl =
        //                    type_instantiation_decl->getAliasedType()
        //                        ->getAs<clang::TemplateSpecializationType>();
        //        }

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
        else {
            const auto target_id = common::to_id(*type->getAsCXXRecordDecl());
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

    parameter.set_type(common::to_string(p.getType(), p.getASTContext()));

    if (p.hasDefaultArg()) {
        const auto *default_arg = p.getDefaultArg();
        if (default_arg != nullptr) {
            auto default_arg_str = common::get_source_text(
                default_arg->getSourceRange(), source_manager());
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

                LOG_DBG("Adding function parameter relationship from {} to "
                        "{}: {}",
                    c.full_name(), clanguml::common::model::to_string(r.type()),
                    r.label());

                c.add_relationship(std::move(r));
            }
        }

        // Also consider the container itself if it is a template
        // instantiation it's arguments could count as reference to relevant
        // types
        auto underlying_type = p.getType();
        if (underlying_type->isReferenceType())
            underlying_type = underlying_type.getNonReferenceType();
        if (underlying_type->isPointerType())
            underlying_type = underlying_type->getPointeeType();

        if (const auto *tsp =
                underlying_type->getAs<clang::TemplateSpecializationType>();
            tsp != nullptr) {
            process_function_parameter_find_relationships_in_template(
                c, template_parameter_names, *tsp);
        }
    }

    method.add_parameter(std::move(parameter));
}

void translation_unit_visitor::
    process_function_parameter_find_relationships_in_template(class_ &c,
        const std::set<std::string> & /*template_parameter_names*/,
        const clang::TemplateSpecializationType &template_instantiation_type)
{
    const auto template_field_decl_name =
        template_instantiation_type.getTemplateName()
            .getAsTemplateDecl()
            ->getQualifiedNameAsString();

    auto template_specialization_ptr =
        build_template_instantiation(template_instantiation_type);

    if (diagram().should_include(template_field_decl_name)) {
        if (template_instantiation_type.isDependentType()) {
            if (template_specialization_ptr) {
                relationship r{relationship_t::kDependency,
                    template_specialization_ptr->id()};

                c.add_relationship(std::move(r));
            }
        }
        else {
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
    auto type_name =
        common::to_string(field_type, field_declaration.getASTContext());
    if (type_name.empty())
        type_name = "<<anonymous>>";

    class_member field{
        common::access_specifier_to_access_t(field_declaration.getAccess()),
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

    set_ast_local_id(cls->getID(), template_instantiation.id());

    return c_ptr;
}

void translation_unit_visitor::process_template_specialization_argument(
    const clang::ClassTemplateSpecializationDecl *cls,
    class_ &template_instantiation, const clang::TemplateArgument &arg,
    size_t argument_index, bool /*in_parameter_pack*/)
{
    const auto argument_kind = arg.getKind();

    if (argument_kind == clang::TemplateArgument::Type) {
        template_parameter argument;
        argument.is_template_parameter(false);

        // If this is a nested template type - add nested templates as
        // template arguments
        if (const auto *nested_template_type =
                arg.getAsType()->getAs<clang::TemplateSpecializationType>();
            nested_template_type != nullptr) {

            const auto nested_template_name =
                nested_template_type->getTemplateName()
                    .getAsTemplateDecl()
                    ->getQualifiedNameAsString();

            argument.set_name(nested_template_name);

            auto nested_template_instantiation = build_template_instantiation(
                *nested_template_type, {&template_instantiation});

            argument.set_id(nested_template_instantiation->id());

            for (const auto &t : nested_template_instantiation->templates())
                argument.add_template_param(t);

            // Check if this template should be simplified (e.g. system
            // template aliases such as 'std:basic_string<char>' should be
            // simply 'std::string')
            simplify_system_template(argument,
                argument.to_string(config().using_namespace(), false));
        }
        else if (arg.getAsType()->getAs<clang::TemplateTypeParmType>() !=
            nullptr) {
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

        LOG_DBG("Adding template instantiation argument {}",
            argument.to_string(config().using_namespace(), false));

        simplify_system_template(
            argument, argument.to_string(config().using_namespace(), false));

        template_instantiation.add_template(std::move(argument));
    }
    else if (argument_kind == clang::TemplateArgument::Integral) {
        template_parameter argument;
        argument.is_template_parameter(false);
        argument.set_type(std::to_string(arg.getAsIntegral().getExtValue()));
        template_instantiation.add_template(std::move(argument));
    }
    else if (argument_kind == clang::TemplateArgument::Expression) {
        template_parameter argument;
        argument.is_template_parameter(false);
        argument.set_type(common::get_source_text(
            arg.getAsExpr()->getSourceRange(), source_manager()));
        template_instantiation.add_template(std::move(argument));
    }
    else if (argument_kind == clang::TemplateArgument::TemplateExpansion) {
        template_parameter argument;
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
        LOG_ERROR("Unsupported template argument kind {} [{}]", arg.getKind(),
            cls->getLocation().printToString(source_manager()));
    }
}

void translation_unit_visitor::
    process_unexposed_template_specialization_parameters(
        const std::string &type_name, template_parameter &tp, class_ &c)
{
    auto template_params = cx::util::parse_unexposed_template_params(
        type_name, [](const std::string &t) { return t; });

    found_relationships_t relationships;
    for (auto &param : template_params) {
        find_relationships_in_unexposed_template_params(param, relationships);
        tp.add_template_param(param);
    }

    for (auto &r : relationships) {
        c.add_relationship({std::get<1>(r), std::get<0>(r)});
    }
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
            element_opt.value().id(), relationship_t::kDependency);
        found = true;
    }

    for (const auto &nested_template_params : ct.template_params()) {
        found = find_relationships_in_unexposed_template_params(
                    nested_template_params, relationships) ||
            found;
    }
    return found;
}

std::unique_ptr<class_> translation_unit_visitor::
    build_template_instantiation_from_class_template_specialization(
        const clang::ClassTemplateSpecializationDecl &template_specialization,
        const clang::RecordType &record_type,
        std::optional<clanguml::class_diagram::model::class_ *> parent)
{
    auto template_instantiation_ptr =
        std::make_unique<class_>(config_.using_namespace());

    //
    // Here we'll hold the template base params to replace with the
    // instantiated values
    //
    std::deque<std::tuple</*parameter name*/ std::string, /* position */ int,
        /*is variadic */ bool>>
        template_base_params{};

    auto &template_instantiation = *template_instantiation_ptr;
    std::string full_template_specialization_name =
        common::to_string(record_type, template_specialization.getASTContext());

    const auto *template_decl =
        template_specialization.getSpecializedTemplate();

    auto qualified_name = template_decl->getQualifiedNameAsString();

    namespace_ ns{qualified_name};
    ns.pop_back();
    template_instantiation.set_name(template_decl->getNameAsString());
    template_instantiation.set_namespace(ns);
    template_instantiation.set_id(template_decl->getID() +
        static_cast<id_t>(
            std::hash<std::string>{}(full_template_specialization_name) >> 4U));

    build_template_instantiation_process_template_arguments(parent,
        template_base_params,
        template_specialization.getTemplateArgs().asArray(),
        template_instantiation, full_template_specialization_name,
        template_decl);

    // First try to find the best match for this template in partially
    // specialized templates
    std::string destination{};
    std::string best_match_full_name{};
    auto full_template_name = template_instantiation.full_name(false);
    int best_match{};
    common::model::diagram_element::id_t best_match_id{0};

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

    auto templated_decl_id = template_specialization.getID();
    auto templated_decl_local_id =
        get_ast_local_id(templated_decl_id).value_or(0);

    if (best_match_id > 0) {
        destination = best_match_full_name;
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, best_match_id});
    }
    // If we can't find optimal match for parent template specialization,
    // just use whatever clang suggests
    else if (diagram().has_element(templated_decl_local_id)) {
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, templated_decl_local_id});
    }
    else if (diagram().should_include(qualified_name)) {
        LOG_DBG("Skipping instantiation relationship from {} to {}",
            template_instantiation_ptr->full_name(false), templated_decl_id);
    }

    return template_instantiation_ptr;
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

    const auto *template_type_ptr = &template_type_decl;

    if (template_type_decl.isTypeAlias()) {
        if (const auto *tsp = template_type_decl.getAliasedType()
                                  ->getAs<clang::TemplateSpecializationType>();
            tsp != nullptr)
            template_type_ptr = tsp;
    }

    const auto &template_type = *template_type_ptr;

    //
    // Create class_ instance to hold the template instantiation
    //
    auto template_instantiation_ptr =
        std::make_unique<class_>(config_.using_namespace());
    auto &template_instantiation = *template_instantiation_ptr;
    std::string full_template_specialization_name = common::to_string(
        template_type.desugar(),
        template_type.getTemplateName().getAsTemplateDecl()->getASTContext());

    auto *template_decl{template_type.getTemplateName().getAsTemplateDecl()};

    auto template_decl_qualified_name =
        template_decl->getQualifiedNameAsString();

    auto *class_template_decl{
        clang::dyn_cast<clang::ClassTemplateDecl>(template_decl)};

    if ((class_template_decl != nullptr) &&
        (class_template_decl->getTemplatedDecl() != nullptr) &&
        (class_template_decl->getTemplatedDecl()->getParent() != nullptr) &&
        class_template_decl->getTemplatedDecl()->getParent()->isRecord()) {

        namespace_ ns{
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
        namespace_ ns{template_decl_qualified_name};
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

    if ((templated_class_decl != nullptr) &&
        templated_class_decl->hasDefinition())
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

    auto templated_decl_id =
        template_type.getTemplateName().getAsTemplateDecl()->getID();
    auto templated_decl_local_id =
        get_ast_local_id(templated_decl_id).value_or(0);

    if (best_match_id > 0) {
        destination = best_match_full_name;
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, best_match_id});
    }
    // If we can't find optimal match for parent template specialization,
    // just use whatever clang suggests
    else if (diagram().has_element(templated_decl_local_id)) {
        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, templated_decl_local_id});
    }
    else {
        LOG_DBG("== Cannot determine global id for specialization template {} "
                "- delaying until the translation unit is complete ",
            templated_decl_id);

        template_instantiation.add_relationship(
            {relationship_t::kInstantiation, templated_decl_id});
    }

    template_instantiation.set_id(
        common::to_id(template_instantiation_ptr->full_name(false)));

    return template_instantiation_ptr;
}

void translation_unit_visitor::
    build_template_instantiation_process_template_arguments(
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        std::deque<std::tuple<std::string, int, bool>> &template_base_params,
        const clang::ArrayRef<clang::TemplateArgument> &template_args,
        class_ &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl)
{
    auto arg_index = 0;
    for (const auto &arg : template_args) {
        const auto argument_kind = arg.getKind();
        template_parameter argument;
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
            LOG_ERROR("Unsupported argument type {}", arg.getKind());
        }

        // We can figure this only when we encounter variadic param in
        // the list of template params, from then this variable is true
        // and we can process following template parameters as belonging
        // to the variadic tuple
        [[maybe_unused]] auto variadic_params{false};

        // In case any of the template arguments are base classes, add
        // them as parents of the current template instantiation class
        if (!template_base_params.empty()) {
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
}

void translation_unit_visitor::
    build_template_instantiation_process_template_argument(
        const clang::TemplateArgument &arg, template_parameter &argument) const
{
    argument.is_template_parameter(true);
    auto arg_name =
        arg.getAsTemplate().getAsTemplateDecl()->getQualifiedNameAsString();
    argument.set_type(arg_name);
}

void translation_unit_visitor::
    build_template_instantiation_process_type_argument(
        std::optional<clanguml::class_diagram::model::class_ *> &parent,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg, class_ &template_instantiation,
        template_parameter &argument)
{
    assert(arg.getKind() == clang::TemplateArgument::Type);

    argument.is_template_parameter(false);

    // If this is a nested template type - add nested templates as
    // template arguments
    if (const auto *function_type =
            arg.getAsType()->getAs<clang::FunctionProtoType>();
        function_type != nullptr) {

        for (const auto &param_type : function_type->param_types()) {
            const auto *param_record_type =
                param_type->getAs<clang::RecordType>();
            if (param_record_type == nullptr)
                continue;

            auto *classTemplateSpecialization =
                llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(
                    param_type->getAsRecordDecl());

            if (classTemplateSpecialization != nullptr) {
                // Read arg info as needed.
                auto nested_template_instantiation =
                    build_template_instantiation_from_class_template_specialization(
                        *classTemplateSpecialization, *param_record_type,
                        diagram().should_include(
                            full_template_specialization_name)
                            ? std::make_optional(&template_instantiation)
                            : parent);

                const auto nested_template_name =
                    classTemplateSpecialization->getQualifiedNameAsString();

                if (nested_template_instantiation) {
                    if (parent.has_value())
                        parent.value()->add_relationship(
                            {relationship_t::kDependency,
                                nested_template_instantiation->id()});
                }

                auto nested_template_instantiation_full_name =
                    nested_template_instantiation->full_name(false);
                if (diagram().should_include(
                        nested_template_instantiation_full_name)) {
                    diagram().add_class(
                        std::move(nested_template_instantiation));
                }
            }
        }
    }
    else if (const auto *nested_template_type =
                 arg.getAsType()->getAs<clang::TemplateSpecializationType>();
             nested_template_type != nullptr) {

        const auto nested_template_name =
            nested_template_type->getTemplateName()
                .getAsTemplateDecl()
                ->getQualifiedNameAsString();

        argument.set_name(nested_template_name);

        auto nested_template_instantiation =
            build_template_instantiation(*nested_template_type,
                diagram().should_include(full_template_specialization_name)
                    ? std::make_optional(&template_instantiation)
                    : parent);

        argument.set_id(nested_template_instantiation->id());

        for (const auto &t : nested_template_instantiation->templates())
            argument.add_template_param(t);

        // Check if this template should be simplified (e.g. system
        // template aliases such as 'std:basic_string<char>' should
        // be simply 'std::string')
        simplify_system_template(
            argument, argument.to_string(config().using_namespace(), false));

        if (nested_template_instantiation &&
            diagram().should_include(
                nested_template_instantiation->full_name(false))) {
            if (diagram().should_include(full_template_specialization_name)) {
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
        if (diagram().should_include(nested_template_instantiation_full_name)) {
            diagram().add_class(std::move(nested_template_instantiation));
        }
    }
    else if (arg.getAsType()->getAs<clang::TemplateTypeParmType>() != nullptr) {
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

void translation_unit_visitor::
    build_template_instantiation_process_integral_argument(
        const clang::TemplateArgument &arg, template_parameter &argument) const
{
    assert(arg.getKind() == clang::TemplateArgument::Integral);

    argument.is_template_parameter(false);
    argument.set_type(std::to_string(arg.getAsIntegral().getExtValue()));
}

void translation_unit_visitor::
    build_template_instantiation_process_expression_argument(
        const clang::TemplateArgument &arg, template_parameter &argument) const
{
    assert(arg.getKind() == clang::TemplateArgument::Expression);

    argument.is_template_parameter(false);
    argument.set_type(common::get_source_text(
        arg.getAsExpr()->getSourceRange(), source_manager()));
}

void translation_unit_visitor::
    build_template_instantiation_process_tag_argument(
        class_ &template_instantiation,
        const std::string &full_template_specialization_name,
        const clang::TemplateDecl *template_decl,
        const clang::TemplateArgument &arg, template_parameter &argument)
{
    assert(arg.getKind() == clang::TemplateArgument::Type);

    argument.is_template_parameter(false);

    argument.set_name(
        common::to_string(arg.getAsType(), template_decl->getASTContext()));

    if (const auto *record_type = arg.getAsType()->getAs<clang::RecordType>();
        record_type != nullptr) {
        if (const auto *record_type_decl = record_type->getAsRecordDecl();
            record_type_decl != nullptr) {
            argument.set_id(common::to_id(arg));

            if (diagram().should_include(full_template_specialization_name)) {
                // Add dependency relationship to the parent
                // template
                template_instantiation.add_relationship(
                    {relationship_t::kDependency, common::to_id(arg)});
            }
        }
    }
    else if (const auto *enum_type = arg.getAsType()->getAs<clang::EnumType>();
             enum_type != nullptr) {
        if (enum_type->getAsTagDecl() != nullptr) {
            template_instantiation.add_relationship(
                {relationship_t::kDependency, common::to_id(arg)});
        }
    }
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
    auto type_name =
        common::to_string(field_type, field_declaration.getASTContext());
    // The field name
    const auto field_name = field_declaration.getNameAsString();

    class_member field{
        common::access_specifier_to_access_t(field_declaration.getAccess()),
        field_name,
        common::to_string(
            field_type, field_declaration.getASTContext(), false)};

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
        // Skip types which are template template parameters of the parent
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

        // Build the template instantiation for the field type
        auto template_specialization_ptr =
            build_template_instantiation(*template_field_type, {&c});

        if (!field.skip_relationship() && template_specialization_ptr) {
            const auto &template_specialization = *template_specialization_ptr;

            // Check if this template instantiation should be added to the
            // current diagram. Even if the top level template type for
            // this instantiation should not be part of the diagram, e.g.
            // it's a std::vector<>, it's nested types might be added
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

                // Add any relationships to the class 'c' to the diagram,
                // unless the top level type has been added as aggregation
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
        if (!template_instantiation_added_as_aggregation) {
            if ((field_type->getAsCXXRecordDecl() != nullptr) &&
                field_type->getAsCXXRecordDecl()->getNameAsString().empty()) {
                // Relationships to fields whose type is an anonymous nested
                // struct have to be handled separately here
                anonymous_struct_relationships_[field_type->getAsCXXRecordDecl()
                                                    ->getID()] =
                    std::make_tuple(
                        field.name(), relationship_hint, field.access());
            }
            else
                find_relationships(
                    field_type, relationships, relationship_hint);
        }

        add_relationships(c, field, relationships);
    }

    c.add_member(std::move(field));
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

void translation_unit_visitor::resolve_local_to_global_ids()
{
    // TODO: Refactor to a map with relationships attached to references
    //       to elements
    for (const auto &cls : diagram().classes()) {
        for (auto &rel : cls.get().relationships()) {
            if (rel.type() == relationship_t::kInstantiation) {
                const auto maybe_local_id = rel.destination();
                if (get_ast_local_id(maybe_local_id)) {
                    LOG_DBG("= Resolved instantiation destination from local "
                            "id {} to global id {}",
                        maybe_local_id, *get_ast_local_id(maybe_local_id));
                    rel.set_destination(*get_ast_local_id(maybe_local_id));
                }
            }
        }
    }
}

void translation_unit_visitor::finalize()
{
    add_incomplete_forward_declarations();
    resolve_local_to_global_ids();
}

bool translation_unit_visitor::simplify_system_template(
    template_parameter &ct, const std::string &full_name) const
{
    if (config().type_aliases().count(full_name) > 0) {
        ct.set_name(config().type_aliases().at(full_name));
        ct.clear_params();
        return true;
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
} // namespace clanguml::class_diagram::visitor
