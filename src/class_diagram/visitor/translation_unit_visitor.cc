/**
 * @file src/class_diagram/visitor/translation_unit_visitor.cc
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

#include <clang/AST/ExprConcepts.h>
#include <clang/Basic/FileManager.h>
#include <clang/Lex/Preprocessor.h>
#include <spdlog/spdlog.h>

namespace clanguml::class_diagram::visitor {

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::class_diagram::model::diagram &diagram,
    const clanguml::config::class_diagram &config)
    : visitor_specialization_t{sm, diagram, config}
    , template_builder_{diagram, config, *this}
{
}

std::unique_ptr<class_> translation_unit_visitor::create_element(
    const clang::NamedDecl *decl) const
{
    auto cls = std::make_unique<class_>(config().using_namespace());
    cls->is_struct(common::is_struct(decl));
    return cls;
}

bool translation_unit_visitor::VisitNamespaceDecl(clang::NamespaceDecl *ns)
{
    assert(ns != nullptr);

    if (config().package_type() == config::package_type_t::kDirectory)
        return true;

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
    id_mapper().add(ns->getID(), p->id());

    if (config().filter_mode() == config::filter_mode_t::advanced ||
        (diagram().should_include(*p) && !diagram().get(p->id()))) {
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
            diagram().add(package_path, std::move(p));
        }
    }

    return true;
}

bool translation_unit_visitor::VisitTypedefDecl(clang::TypedefDecl *decl)
{
    if (const auto *enm = common::get_typedef_enum_decl(decl)) {
        // Associate a typedef with an anonymous declaration so that it can
        // be later used to assign proper name to enum
        if (!should_include(enm))
            return true;

        LOG_DBG("= Visiting typedef enum declaration {} at {}",
            enm->getQualifiedNameAsString(),
            enm->getLocation().printToString(source_manager()));

        auto e_ptr = create_enum_declaration(enm, decl);

        if (e_ptr && diagram().should_include(*e_ptr))
            add_enum(std::move(e_ptr));
    }

    return true; // Continue traversing
}

bool translation_unit_visitor::VisitEnumDecl(clang::EnumDecl *enm)
{
    assert(enm != nullptr);

    // Anonymous enum values are either class fields with type enum or
    // `typedef enum` declarations
    if (enm->getNameAsString().empty()) {
        return true;
    }

    if (!should_include(enm))
        return true;

    LOG_DBG("= Visiting enum declaration {} at {}",
        enm->getQualifiedNameAsString(),
        enm->getLocation().printToString(source_manager()));

    auto e_ptr = create_enum_declaration(enm, nullptr);

    if (e_ptr && diagram().should_include(*e_ptr))
        add_enum(std::move(e_ptr));

    return true;
}

std::unique_ptr<clanguml::class_diagram::model::enum_>
translation_unit_visitor::create_enum_declaration(
    const clang::EnumDecl *enm, const clang::TypedefDecl *typedef_decl)
{
    auto e_ptr = std::make_unique<enum_>(config().using_namespace());
    auto &e = *e_ptr;

    auto ns{common::get_tag_namespace(*enm)};

    // Id of parent class or struct in which this enum is potentially nested
    std::optional<eid_t> parent_id_opt;
    [[maybe_unused]] namespace_ parent_ns;
    find_record_parent_id(enm, parent_id_opt, parent_ns);

    std::string enm_name;
    if (enm->getNameAsString().empty() && typedef_decl != nullptr)
        enm_name = typedef_decl->getNameAsString();
    else if (parent_id_opt)
        enm_name = enm->getNameAsString();
    else
        enm_name = common::get_tag_name(*enm);

    if (parent_id_opt && diagram().find<class_>(*parent_id_opt)) {
        auto parent_class = diagram().find<class_>(*parent_id_opt);

        e.set_namespace(ns);
        e.set_name(parent_class.value().name(), enm_name);
        e.set_id(common::to_id(e.full_name(false)));
        e.add_relationship({relationship_t::kContainment, *parent_id_opt});
        e.nested(true);
    }
    else if (parent_id_opt && diagram().find<objc_interface>(*parent_id_opt)) {
        auto parent_class = diagram().find<objc_interface>(*parent_id_opt);

        e.set_namespace(ns);
        e.set_name(parent_class.value().name(), enm_name);
        e.set_id(common::to_id(e.full_name(false)));
        e.add_relationship({relationship_t::kContainment, *parent_id_opt});
        e.nested(true);
    }
    else {
        e.set_name(enm_name);
        e.set_namespace(ns);
        e.set_id(common::to_id(e.full_name(false)));
    }

    id_mapper().add(enm->getID(), e.id());

    process_comment(*enm, e);
    set_source_location(*enm, e);
    set_owning_module(*enm, e);

    if (e.skip())
        return {};

    e.set_style(e.style_spec());

    for (const auto &ev : enm->enumerators()) {
        e.constants().push_back(ev->getNameAsString());
    }

    return e_ptr;
}

bool translation_unit_visitor::VisitClassTemplateSpecializationDecl(
    clang::ClassTemplateSpecializationDecl *cls)
{
    if (!should_include(cls))
        return true;

    LOG_DBG("= Visiting template specialization declaration {} at {} "
            "(described class id {})",
        cls->getQualifiedNameAsString(),
        cls->getLocation().printToString(source_manager()),
        cls->getSpecializedTemplate()
            ? cls->getSpecializedTemplate()->getTemplatedDecl()->getID()
            : 0);

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass() != nullptr)
        return true;

    auto template_specialization_ptr = process_template_specialization(cls);

    if (!template_specialization_ptr)
        return true;

    auto &template_specialization = *template_specialization_ptr;

    if (cls->hasDefinition()) {
        // Process template specialization bases
        process_class_bases(cls, template_specialization);

        // Process class child entities
        process_class_children(cls, template_specialization);
    }

    if (!template_specialization.template_specialization_found()) {
        // Only do this if we haven't found a better specialization during
        // construction of the template specialization
        const eid_t ast_id{cls->getSpecializedTemplate()->getID()};
        const auto maybe_id = id_mapper().get_global_id(ast_id);
        if (maybe_id.has_value())
            template_specialization.add_relationship(
                {relationship_t::kInstantiation, maybe_id.value()});
    }

    if (diagram().should_include(template_specialization)) {
        const auto full_name = template_specialization.full_name(false);
        const auto id = template_specialization.id();

        LOG_DBG("Adding class template specialization {} with id {}", full_name,
            id);
        add_class(std::move(template_specialization_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitTypeAliasTemplateDecl(
    clang::TypeAliasTemplateDecl *cls)
{
    if (!should_include(cls))
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
        std::make_unique<class_>(config().using_namespace());
    tbuilder().build_from_template_specialization_type(
        *template_specialization_ptr, cls, *template_type_specialization_ptr);

    template_specialization_ptr->is_template(true);

    if (diagram().should_include(*template_specialization_ptr)) {
        const auto name = template_specialization_ptr->full_name(true);
        const auto id = template_specialization_ptr->id();

        LOG_DBG("Adding class {} with id {}", name, id);

        set_source_location(*cls, *template_specialization_ptr);
        set_owning_module(*cls, *template_specialization_ptr);

        add_class(std::move(template_specialization_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitClassTemplateDecl(
    clang::ClassTemplateDecl *cls)
{
    if (!should_include(cls))
        return true;

    LOG_DBG("= Visiting class template declaration {} at {}",
        cls->getQualifiedNameAsString(),
        cls->getLocation().printToString(source_manager()));

    auto c_ptr = create_class_declaration(cls->getTemplatedDecl());

    if (!c_ptr)
        return true;

    add_processed_template_class(cls->getQualifiedNameAsString());

    tbuilder().build_from_template_declaration(*c_ptr, *cls, *c_ptr);

    // Override the id with the template id, for now we don't care about the
    // underlying templated class id
    const auto cls_full_name = c_ptr->full_name(false);
    const auto id = common::to_id(cls_full_name);

    c_ptr->set_id(id);
    c_ptr->is_template(true);

    id_mapper().add(cls->getID(), id);

    constexpr auto kMaxConstraintCount = 24U;
    llvm::SmallVector<const clang::Expr *, kMaxConstraintCount> constraints{};
    if (cls->hasAssociatedConstraints()) {
        cls->getAssociatedConstraints(constraints);
    }

    for (const auto *expr : constraints) {
        find_relationships_in_constraint_expression(*c_ptr, expr);
    }

    if (!cls->getTemplatedDecl()->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(c_ptr));
        return true;
    }
    process_class_declaration(*cls->getTemplatedDecl(), *c_ptr);
    forward_declarations_.erase(id);

    if (diagram().should_include(*c_ptr)) {
        const auto name = c_ptr->full_name(true);
        LOG_DBG("Adding class template {} with id {}", name, id);

        add_class(std::move(c_ptr));
    }

    return true;
}

bool translation_unit_visitor::VisitRecordDecl(clang::RecordDecl *rec)
{
    if (clang::dyn_cast_or_null<clang::CXXRecordDecl>(rec) != nullptr)
        // This is handled by VisitCXXRecordDecl()
        return true;

    // It seems we are in a C (not C++) translation unit
    if (!should_include(rec))
        return true;

    LOG_DBG("= Visiting record declaration {} at {}",
        rec->getQualifiedNameAsString(),
        rec->getLocation().printToString(source_manager()));

    auto record_ptr = create_record_declaration(rec);

    if (!record_ptr)
        return true;

    const auto rec_id = record_ptr->id();

    id_mapper().add(rec->getID(), rec_id);

    auto &record_model = diagram().find<class_>(rec_id).has_value()
        ? *diagram().find<class_>(rec_id).get()
        : *record_ptr;

    if (rec->isCompleteDefinition() && !record_model.complete()) {
        process_record_members(rec, record_model);
        record_model.complete(true);
    }

    auto id = record_model.id();
    if (!rec->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(record_ptr));
        return true;
    }
    forward_declarations_.erase(id);

    if (diagram().should_include(record_model)) {
        LOG_DBG("Adding struct/union {} with id {}",
            record_model.full_name(false), record_model.id());

        add_class(std::move(record_ptr));
    }
    else {
        LOG_DBG("Skipping struct/union {} with id {}",
            record_model.full_name(true), record_model.id());
    }

    return true;
}

bool translation_unit_visitor::VisitObjCCategoryDecl(
    clang::ObjCCategoryDecl *decl)
{
    if (!should_include(decl))
        return true;

    LOG_DBG("= Visiting ObjC category declaration {} at {}",
        decl->getQualifiedNameAsString(),
        decl->getLocation().printToString(source_manager()));

    auto category_ptr = create_objc_category_declaration(decl);

    if (!category_ptr)
        return true;

    const auto category_id = category_ptr->id();

    id_mapper().add(decl->getID(), category_id);

    auto &category_model =
        diagram().find<objc_interface>(category_id).has_value()
        ? *diagram().find<objc_interface>(category_id).get()
        : *category_ptr;

    process_objc_category_declaration(*decl, category_model);

    if (diagram().should_include(category_model)) {
        LOG_DBG("Adding ObjC category {} with id {}",
            category_model.full_name(false), category_model.id());

        add_objc_interface(std::move(category_ptr));
    }
    else {
        LOG_DBG("Skipping ObjC category {} with id {}",
            category_model.full_name(true), category_model.id());
    }

    return true;
}

bool translation_unit_visitor::VisitObjCProtocolDecl(
    clang::ObjCProtocolDecl *decl)
{
    if (!should_include(decl))
        return true;

    LOG_DBG("= Visiting ObjC protocol declaration {} at {}",
        decl->getQualifiedNameAsString(),
        decl->getLocation().printToString(source_manager()));

    auto protocol_ptr = create_objc_protocol_declaration(decl);

    if (!protocol_ptr)
        return true;

    const auto protocol_id = protocol_ptr->id();

    id_mapper().add(decl->getID(), protocol_id);

    auto &protocol_model =
        diagram().find<objc_interface>(protocol_id).has_value()
        ? *diagram().find<objc_interface>(protocol_id).get()
        : *protocol_ptr;

    process_objc_protocol_declaration(*decl, protocol_model);

    if (diagram().should_include(protocol_model)) {
        LOG_DBG("Adding ObjC protocol {} with id {}",
            protocol_model.full_name(false), protocol_model.id());

        add_objc_interface(std::move(protocol_ptr));
    }
    else {
        LOG_DBG("Skipping ObjC protocol {} with id {}",
            protocol_model.full_name(true), protocol_model.id());
    }

    return true;
}

bool translation_unit_visitor::VisitObjCInterfaceDecl(
    clang::ObjCInterfaceDecl *decl)
{
    if (!should_include(decl))
        return true;

    LOG_DBG("= Visiting ObjC interface declaration {} at {}",
        decl->getQualifiedNameAsString(),
        decl->getLocation().printToString(source_manager()));

    auto interface_ptr = create_objc_interface_declaration(decl);

    if (!interface_ptr)
        return true;

    const auto protocol_id = interface_ptr->id();

    id_mapper().add(decl->getID(), protocol_id);

    auto &interface_model =
        diagram().find<objc_interface>(protocol_id).has_value()
        ? *diagram().find<objc_interface>(protocol_id).get()
        : *interface_ptr;

    if (!interface_model.complete())
        process_objc_interface_declaration(*decl, interface_model);

    if (diagram().should_include(interface_model)) {
        LOG_DBG("Adding ObjC interface {} with id {}",
            interface_model.full_name(false), interface_model.id());

        add_objc_interface(std::move(interface_ptr));
    }
    else {
        LOG_DBG("Skipping ObjC interface {} with id {}",
            interface_model.full_name(true), interface_model.id());
    }

    return true;
}

bool translation_unit_visitor::TraverseConceptDecl(clang::ConceptDecl *cpt)
{
    if (!should_include(cpt))
        return true;

    LOG_DBG("= Visiting concept (isType: {}) declaration {} at {}",
        cpt->isTypeConcept(), cpt->getQualifiedNameAsString(),
        cpt->getLocation().printToString(source_manager()));

    auto concept_model = create_concept_declaration(cpt);

    if (!concept_model)
        return true;

    const auto concept_id = concept_model->id();

    id_mapper().add(cpt->getID(), concept_id);

    tbuilder().build_from_template_declaration(*concept_model, *cpt);

    constexpr auto kMaxConstraintCount = 24U;
    llvm::SmallVector<const clang::Expr *, kMaxConstraintCount> constraints{};
    if (cpt->hasAssociatedConstraints()) {
        cpt->getAssociatedConstraints(constraints);
    }

    for (const auto *expr : constraints) {
        find_relationships_in_constraint_expression(*concept_model, expr);
    }

    if (cpt->getConstraintExpr() != nullptr) {
        process_constraint_requirements(
            cpt, cpt->getConstraintExpr(), *concept_model);

        find_relationships_in_constraint_expression(
            *concept_model, cpt->getConstraintExpr());
    }

    if (diagram().should_include(*concept_model)) {
        LOG_DBG("Adding concept {} with id {}", concept_model->full_name(false),
            concept_model->id());

        add_concept(std::move(concept_model));
    }
    else {
        LOG_DBG("Skipping concept {} with id {}",
            concept_model->full_name(true), concept_model->id());
    }

    return true;
}

void translation_unit_visitor::process_constraint_requirements(
    const clang::ConceptDecl *cpt, const clang::Expr *expr,
    model::concept_ &concept_model) const
{
    if (const auto *constraint = llvm::dyn_cast<clang::RequiresExpr>(expr);
        constraint) {

        auto constraint_source = common::to_string(constraint);

        LOG_DBG("== Processing constraint: '{}'", constraint_source);

        for ([[maybe_unused]] const auto *requirement :
            constraint->getRequirements()) {
            // TODO
        }

        // process 'requires (...)' declaration
        for (const auto *decl : constraint->getBody()->decls()) {
            if (const auto *parm_var_decl =
                    llvm::dyn_cast<clang::ParmVarDecl>(decl);
                parm_var_decl) {
                parm_var_decl->getQualifiedNameAsString();

                auto param_name = parm_var_decl->getNameAsString();
                auto param_type = common::to_string(
                    parm_var_decl->getType(), cpt->getASTContext());

                LOG_DBG("=== Processing parameter variable declaration: {}, {}",
                    param_type, param_name);

                concept_model.add_parameter(
                    {std::move(param_type), std::move(param_name)});
            }
            else {
                LOG_DBG("=== Processing some other concept declaration: {}",
                    decl->getID());
            }
        }

        // process concept body requirements '{ }' if any
        for (const auto *req : constraint->getRequirements()) {
            if (req->getKind() == clang::concepts::Requirement::RK_Simple) {
                const auto *simple_req =
                    llvm::dyn_cast<clang::concepts::ExprRequirement>(req);

                if (simple_req != nullptr) {
                    util::if_not_null(
                        simple_req->getExpr(), [&concept_model](const auto *e) {
                            auto simple_expr = common::to_string(e);

                            LOG_DBG("=== Processing expression requirement: {}",
                                simple_expr);

                            concept_model.add_statement(std::move(simple_expr));
                        });
                }
            }
            else if (req->getKind() == clang::concepts::Requirement::RK_Type) {
                util::if_not_null(
                    llvm::dyn_cast<clang::concepts::TypeRequirement>(req),
                    [&concept_model, cpt](const auto *t) {
                        auto type_name = common::to_string(
                            t->getType()->getType(), cpt->getASTContext());

                        LOG_DBG(
                            "=== Processing type requirement: {}", type_name);

                        concept_model.add_statement(std::move(type_name));
                    });
            }
            else if (req->getKind() ==
                clang::concepts::Requirement::RK_Nested) {
                const auto *nested_req =
                    llvm::dyn_cast<clang::concepts::NestedRequirement>(req);

                if (nested_req != nullptr) {
                    util::if_not_null(
                        nested_req->getConstraintExpr(), [](const auto *e) {
                            LOG_DBG("=== Processing nested requirement: {}",
                                common::to_string(e));
                        });
                }
            }
            else if (req->getKind() ==
                clang::concepts::Requirement::RK_Compound) {
                const auto *compound_req =
                    llvm::dyn_cast<clang::concepts::ExprRequirement>(req);

                if (compound_req != nullptr) {
                    const auto *compound_expr_ptr = compound_req->getExpr();

                    if (compound_expr_ptr != nullptr) {
                        auto compound_expr =
                            common::to_string(compound_expr_ptr);

                        auto req_return_type =
                            compound_req->getReturnTypeRequirement();

                        if (!req_return_type.isEmpty()) {
                            compound_expr =
                                fmt::format("{{{}}} -> {}", compound_expr,
                                    common::to_string(
                                        req_return_type.getTypeConstraint()));
                        }
                        else if (compound_req->hasNoexceptRequirement()) {
                            compound_expr =
                                fmt::format("{{{}}} noexcept", compound_expr);
                        }

                        LOG_DBG("=== Processing compound requirement: {}",
                            compound_expr);

                        concept_model.add_statement(std::move(compound_expr));
                    }
                }
            }
        }
    }
    else if (const auto *binop = llvm::dyn_cast<clang::BinaryOperator>(expr);
             binop) {
        process_constraint_requirements(cpt, binop->getLHS(), concept_model);
        process_constraint_requirements(cpt, binop->getRHS(), concept_model);
    }
    else if (const auto *unop = llvm::dyn_cast<clang::UnaryOperator>(expr);
             unop) {
        process_constraint_requirements(cpt, unop->getSubExpr(), concept_model);
    }
}

void translation_unit_visitor::find_relationships_in_constraint_expression(
    clanguml::common::model::element &c, const clang::Expr *expr)
{
    if (expr == nullptr)
        return;
    found_relationships_t relationships;

    common::if_dyn_cast<clang::UnresolvedLookupExpr>(
        expr, [&](const clang::UnresolvedLookupExpr *ul) {
            for (const auto ta : ul->template_arguments()) {
                if (ta.getArgument().getKind() !=
                    clang::TemplateArgument::ArgKind::Type)
                    continue;
                find_relationships(ta.getArgument().getAsType(), relationships,
                    relationship_t::kConstraint);
            }
        });

    common::if_dyn_cast<clang::ConceptSpecializationExpr>(
        expr, [&](const auto *cs) {
            process_concept_specialization_relationships(c, cs);
        });

    common::if_dyn_cast<clang::RequiresExpr>(expr, [&](const auto *re) {
        // TODO
    });

    common::if_dyn_cast<clang::BinaryOperator>(expr, [&](const auto *op) {
        find_relationships_in_constraint_expression(c, op->getLHS());
        find_relationships_in_constraint_expression(c, op->getRHS());
    });

    common::if_dyn_cast<clang::UnaryOperator>(expr, [&](const auto *op) {
        find_relationships_in_constraint_expression(c, op->getSubExpr());
    });

    for (const auto &[type_element_id, relationship_type] : relationships) {
        if (type_element_id != c.id() &&
            (relationship_type != relationship_t::kNone)) {

            relationship r{relationship_type, type_element_id};

            c.add_relationship(std::move(r));
        }
    }
}

void translation_unit_visitor::process_concept_specialization_relationships(
    common::model::element &c,
    const clang::ConceptSpecializationExpr *concept_specialization)
{

    if (const auto *cpt = concept_specialization->getNamedConcept();
        should_include(cpt)) {

        const auto cpt_name = cpt->getNameAsString();
        const eid_t ast_id{cpt->getID()};
        const auto maybe_id = id_mapper().get_global_id(ast_id);
        if (!maybe_id)
            return;

        const auto target_id = maybe_id.value();

        std::vector<std::string> constrained_template_params;

        size_t argument_index{};

        for (const auto ta : concept_specialization->getTemplateArguments()) {
            if (ta.getKind() == clang::TemplateArgument::Type) {
                auto type_name =
                    common::to_string(ta.getAsType(), cpt->getASTContext());
                extract_constrained_template_param_name(concept_specialization,
                    cpt, constrained_template_params, argument_index,
                    type_name);
            }
            else if (ta.getKind() == clang::TemplateArgument::Pack) {
                if (!ta.getPackAsArray().empty() &&
                    ta.getPackAsArray().front().isPackExpansion()) {
                    const auto &pack_head =
                        ta.getPackAsArray().front().getAsType();
                    auto type_name =
                        common::to_string(pack_head, cpt->getASTContext());
                    extract_constrained_template_param_name(
                        concept_specialization, cpt,
                        constrained_template_params, argument_index, type_name);
                }
            }
            else {
                LOG_DBG("Unsupported concept type parameter in concept: {}",
                    cpt_name);
            }
            argument_index++;
        }

        if (!constrained_template_params.empty())
            c.add_relationship(
                {relationship_t::kConstraint, target_id, access_t::kNone,
                    fmt::format(
                        "{}", fmt::join(constrained_template_params, ","))});
    }
}

bool translation_unit_visitor::VisitCXXRecordDecl(clang::CXXRecordDecl *cls)
{
    if (!should_include(cls))
        return true;

    LOG_DBG("= Visiting class declaration {} at {}",
        cls->getQualifiedNameAsString(),
        cls->getLocation().printToString(source_manager()));

    LOG_DBG(
        "== getQualifiedNameAsString() = {}", cls->getQualifiedNameAsString());
    if (cls->getOwningModule() != nullptr)
        LOG_DBG(
            "== getOwningModule()->Name = {}", cls->getOwningModule()->Name);
    LOG_DBG("== getID() = {}", cls->getID());
    LOG_DBG("== isTemplateDecl() = {}", cls->isTemplateDecl());
    LOG_DBG("== isTemplated() = {}", cls->isTemplated());
    LOG_DBG("== getParent()->isRecord()() = {}", cls->getParent()->isRecord());

    if (const auto *parent_record =
            clang::dyn_cast<clang::RecordDecl>(cls->getParent());
        parent_record != nullptr) {
        LOG_DBG("== getParent()->getQualifiedNameAsString() = {}",
            parent_record->getQualifiedNameAsString());
    }

    if (has_processed_template_class(cls->getQualifiedNameAsString()))
        // If we have already processed the template of this class
        // skip it
        return true;

    if (cls->isTemplated() && (cls->getDescribedTemplate() != nullptr)) {
        // If the described templated of this class is already in the model
        // skip it:
        const eid_t ast_id{cls->getDescribedTemplate()->getID()};
        if (id_mapper().get_global_id(ast_id))
            return true;
    }

    // TODO: Add support for classes defined in function/method bodies
    if (cls->isLocalClass() != nullptr)
        return true;

    auto c_ptr = create_class_declaration(cls);

    if (!c_ptr)
        return true;

    const auto cls_id = c_ptr->id();

    id_mapper().add(cls->getID(), cls_id);

    auto &class_model = diagram().find<class_>(cls_id).has_value()
        ? *diagram().find<class_>(cls_id).get()
        : *c_ptr;

    if (cls->isCompleteDefinition() && !class_model.complete())
        process_class_declaration(*cls, class_model);

    auto id = class_model.id();
    if (!cls->isCompleteDefinition()) {
        forward_declarations_.emplace(id, std::move(c_ptr));
        return true;
    }
    forward_declarations_.erase(id);

    if (diagram().should_include(class_model)) {
        LOG_DBG("Adding class {} with id {}", class_model, class_model.id());

        add_class(std::move(c_ptr));
    }
    else {
        LOG_DBG("Skipping class {} with id {}", class_model, class_model.id());
    }

    return true;
}

std::unique_ptr<clanguml::class_diagram::model::concept_>
translation_unit_visitor::create_concept_declaration(clang::ConceptDecl *cpt)
{
    assert(cpt != nullptr);

    if (!should_include(cpt))
        return {};

    auto concept_ptr{
        std::make_unique<model::concept_>(config().using_namespace())};
    auto &concept_model = *concept_ptr;

    auto ns = common::get_template_namespace(*cpt);

    concept_model.set_name(cpt->getNameAsString());
    concept_model.set_namespace(ns);
    concept_model.set_id(common::to_id(concept_model.full_name(false)));

    process_comment(*cpt, concept_model);
    set_source_location(*cpt, concept_model);
    set_owning_module(*cpt, concept_model);

    if (concept_model.skip())
        return {};

    concept_model.set_style(concept_model.style_spec());

    return concept_ptr;
}

std::unique_ptr<class_> translation_unit_visitor::create_record_declaration(
    clang::RecordDecl *rec)
{
    assert(rec != nullptr);

    if (!should_include(rec))
        return {};

    auto record_ptr{std::make_unique<class_>(config().using_namespace())};
    auto &record = *record_ptr;

    process_record_parent(rec, record, namespace_{});

    if (!record.is_nested()) {
        auto record_name = rec->getQualifiedNameAsString();

#if LLVM_VERSION_MAJOR < 16
        if (record_name == "(anonymous)") {
            util::if_not_null(rec->getTypedefNameForAnonDecl(),
                [&record_name](const clang::TypedefNameDecl *name) {
                    record_name = name->getNameAsString();
                });
        }
#endif

        record.set_name(record_name);
        record.set_id(common::to_id(record.full_name(false)));
    }

    process_comment(*rec, record);
    set_source_location(*rec, record);
    set_owning_module(*rec, record);

    const auto record_full_name = record_ptr->full_name(false);

    record.is_struct(rec->isStruct());
    record.is_union(rec->isUnion());

    if (record.skip())
        return {};

    record.set_style(record.style_spec());

    return record_ptr;
}

std::unique_ptr<class_> translation_unit_visitor::create_class_declaration(
    clang::CXXRecordDecl *cls)
{
    assert(cls != nullptr);

    if (!should_include(cls))
        return {};

    auto c_ptr{std::make_unique<class_>(config().using_namespace())};
    auto &c = *c_ptr;

    auto ns{common::get_tag_namespace(*cls)};

    process_record_parent(cls, c, ns);

    if (!c.is_nested()) {
        c.set_name(common::get_tag_name(*cls));
        c.set_namespace(ns);
        c.set_id(common::to_id(c.full_name(false)));
    }

    c.is_struct(cls->isStruct());

    process_comment(*cls, c);
    set_source_location(*cls, c);
    set_owning_module(*cls, c);

    if (c.skip())
        return {};

    c.set_style(c.style_spec());

    return c_ptr;
}

std::unique_ptr<clanguml::class_diagram::model::objc_interface>
translation_unit_visitor::create_objc_category_declaration(
    clang::ObjCCategoryDecl *decl)
{
    assert(decl != nullptr);

    if (!should_include(decl))
        return {};

    auto c_ptr{std::make_unique<class_diagram::model::objc_interface>(
        config().using_namespace())};
    auto &c = *c_ptr;

    decl->getClassInterface()->getNameAsString();
    c.set_name(fmt::format("{}({})",
        decl->getClassInterface()->getNameAsString(), decl->getNameAsString()));
    c.set_id(common::to_id(fmt::format("__objc__category__{}", c.name())));
    c.is_category(true);

    process_comment(*decl, c);
    set_source_location(*decl, c);

    if (c.skip())
        return {};

    c.set_style(c.style_spec());

    return c_ptr;
}

std::unique_ptr<clanguml::class_diagram::model::objc_interface>
translation_unit_visitor::create_objc_protocol_declaration(
    clang::ObjCProtocolDecl *decl)
{
    assert(decl != nullptr);

    if (!should_include(decl))
        return {};

    auto c_ptr{std::make_unique<class_diagram::model::objc_interface>(
        config().using_namespace())};
    auto &c = *c_ptr;

    c.set_name(decl->getNameAsString());
    c.set_id(common::to_id(*decl));
    c.is_protocol(true);

    process_comment(*decl, c);
    set_source_location(*decl, c);

    if (c.skip())
        return {};

    c.set_style(c.style_spec());

    return c_ptr;
}

std::unique_ptr<clanguml::class_diagram::model::objc_interface>
translation_unit_visitor::create_objc_interface_declaration(
    clang::ObjCInterfaceDecl *decl)
{
    assert(decl != nullptr);

    if (!should_include(decl))
        return {};

    auto c_ptr{std::make_unique<class_diagram::model::objc_interface>(
        config().using_namespace())};
    auto &c = *c_ptr;

    c.set_name(decl->getNameAsString());
    c.set_id(common::to_id(*decl));

    process_comment(*decl, c);
    set_source_location(*decl, c);

    if (c.skip())
        return {};

    c.set_style(c.style_spec());

    return c_ptr;
}

void translation_unit_visitor::process_record_parent(
    clang::RecordDecl *cls, class_ &c, const namespace_ &ns)
{

    std::optional<eid_t> id_opt;
    namespace_ parent_ns = ns;

    find_record_parent_id(cls, id_opt, parent_ns);

    if (id_opt && diagram().find<class_>(*id_opt)) {
        process_record_parent_by_type<class_>(*id_opt, c, parent_ns, cls);
    }

    if (id_opt && diagram().find<objc_interface>(*id_opt)) {
        process_record_parent_by_type<objc_interface>(
            *id_opt, c, parent_ns, cls);
    }
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

void translation_unit_visitor::process_objc_category_declaration(
    const clang::ObjCCategoryDecl &cls, objc_interface &c)
{
    assert(c.is_category());

    // Iterate over class methods (both regular and static)
    for (const auto *method : cls.methods()) {
        if (method != nullptr) {
            process_objc_method(*method, c);
        }
    }

    // Add relationship to the ObjC Interface being extended by this
    // category
    if (cls.getClassInterface() != nullptr) {
        eid_t objc_interface_id = common::to_id(*cls.getClassInterface());
        common::model::relationship r{
            relationship_t::kInstantiation, objc_interface_id, access_t::kNone};

        LOG_DBG("Found protocol {} [{}] for ObjC interface {}",
            cls.getClassInterface()->getNameAsString(),
            objc_interface_id.value(), c.name());

        c.add_relationship(std::move(r));
    }

    c.complete(true);
}

void translation_unit_visitor::process_objc_protocol_declaration(
    const clang::ObjCProtocolDecl &cls, objc_interface &c)
{
    assert(c.is_protocol());

    // Iterate over class methods (both regular and static)
    for (const auto *method : cls.methods()) {
        if (method != nullptr) {
            process_objc_method(*method, c);
        }
    }

    c.complete(true);
}

void translation_unit_visitor::process_objc_interface_declaration(
    const clang::ObjCInterfaceDecl &cls, objc_interface &c)
{
    assert(!c.is_protocol() && !c.is_category());

    // Iterate over class methods (both regular and static)
    for (const auto *method : cls.methods()) {
        if (method != nullptr) {
            process_objc_method(*method, c);
        }
    }

    for (const auto *ivar : cls.ivars()) {
        if (ivar != nullptr) {
            process_objc_ivar(*ivar, c);
        }
    }

    process_objc_interface_base(cls, c);

    c.complete(true);
}
void translation_unit_visitor::process_objc_interface_base(
    const clang::ObjCInterfaceDecl &cls, objc_interface &c)
{
    if (const auto *base = cls.getSuperClass(); base != nullptr) {
        eid_t parent_id = common::to_id(*base);
        common::model::relationship cp{parent_id, access_t::kNone, false};

        LOG_DBG("Found base class {} [{}] for ObjC interface {}",
            base->getNameAsString(), parent_id.value(), c.name());

        c.add_relationship(std::move(cp));
    }

    for (const auto *protocol : cls.protocols()) {
        eid_t parent_id = common::to_id(*protocol);
        common::model::relationship cp{
            relationship_t::kInstantiation, parent_id, access_t::kNone};

        LOG_DBG("Found protocol {} [{}] for ObjC interface {}",
            protocol->getNameAsString(), parent_id.value(), c.name());

        c.add_relationship(std::move(cp));
    }
}

void translation_unit_visitor::process_objc_ivar(
    const clang::ObjCIvarDecl &ivar, objc_interface &c)
{
    LOG_DBG("== Visiting ObjC ivar {}", ivar.getNameAsString());

    // Default hint for relationship is aggregation
    auto relationship_hint = relationship_t::kAggregation;

    auto field_type = ivar.getType();
    auto field_type_str =
        common::to_string(field_type, ivar.getASTContext(), false);

    auto type_name = common::to_string(field_type, ivar.getASTContext());

    const auto field_name = ivar.getNameAsString();

    objc_member field{
        common::access_specifier_to_access_t(ivar.getAccessControl()),
        field_name, field_type_str};

    field.set_qualified_name(ivar.getQualifiedNameAsString());

    process_comment(ivar, field);
    set_source_location(ivar, field);

    if (field.skip())
        return;

    if (field_type->isObjCObjectPointerType()) {
        relationship_hint = relationship_t::kAggregation;
        field_type = field_type->getPointeeType();
    }
    else if (field_type->isPointerType()) {
        relationship_hint = relationship_t::kAssociation;
        field_type = field_type->getPointeeType();
    }
    else if (field_type->isLValueReferenceType()) {
        relationship_hint = relationship_t::kAssociation;
        field_type = field_type.getNonReferenceType();
    }
    else if (field_type->isArrayType()) {
        relationship_hint = relationship_t::kAggregation;
        while (field_type->isArrayType()) {
            auto current_multiplicity = field.destination_multiplicity();
            if (!current_multiplicity)
                field.set_destination_multiplicity(common::get_array_size(
                    *field_type->getAsArrayTypeUnsafe()));
            else {
                auto maybe_array_size =
                    common::get_array_size(*field_type->getAsArrayTypeUnsafe());
                if (maybe_array_size.has_value()) {
                    field.set_destination_multiplicity(
                        current_multiplicity.value() *
                        maybe_array_size.value());
                }
            }

            field_type = field_type->getAsArrayTypeUnsafe()->getElementType();
        }
    }
    else if (field_type->isRValueReferenceType()) {
        field_type = field_type.getNonReferenceType();
    }

    found_relationships_t relationships;

    if (!field.skip_relationship()) {
        // Find relationship for the type if the type has not been added
        // as aggregation
        if (field_type->getAsObjCInterfaceType() != nullptr &&
            field_type->getAsObjCInterfaceType()->getInterface() != nullptr) {
            const auto *objc_iface =
                field_type->getAsObjCInterfaceType()->getInterface();

            relationships.emplace_back(
                common::to_id(*objc_iface), relationship_t::kAggregation);
        }
        else if ((field_type->getAsRecordDecl() != nullptr) &&
            field_type->getAsRecordDecl()->getNameAsString().empty()) {
            // Relationships to fields whose type is an anonymous nested
            // struct have to be handled separately here
            anonymous_struct_relationships_[field_type->getAsRecordDecl()
                                                ->getID()] =
                std::make_tuple(field.name(), relationship_hint, field.access(),
                    field.destination_multiplicity());
        }
        else
            find_relationships(field_type, relationships, relationship_hint);

        add_relationships(c, field, relationships);
    }

    // If this is an anonymous struct - replace the anonymous_XYZ part with
    // field name
    if ((field_type->getAsRecordDecl() != nullptr) &&
        field_type->getAsRecordDecl()->getNameAsString().empty()) {
        if (util::contains(field.type(), "(anonymous_")) {
            std::regex anonymous_re("anonymous_(\\d*)");
            field.set_type(
                std::regex_replace(field.type(), anonymous_re, field_name));
        }
    }

    c.add_member(std::move(field));
}

void translation_unit_visitor::process_class_bases(
    const clang::CXXRecordDecl *cls, class_ &c)
{
    for (const auto &base : cls->bases()) {
        eid_t parent_id;
        if (const auto *tsp =
                base.getType()->getAs<clang::TemplateSpecializationType>();
            tsp != nullptr) {
            auto template_specialization_ptr =
                std::make_unique<class_>(config().using_namespace());
            tbuilder().build_from_template_specialization_type(
                *template_specialization_ptr, cls, *tsp, {});

            parent_id = template_specialization_ptr->id();

            if (diagram().should_include(*template_specialization_ptr)) {
                add_class(std::move(template_specialization_ptr));
            }
        }
        else if (const auto *record_type =
                     base.getType()->getAs<clang::RecordType>();
                 record_type != nullptr) {
            parent_id = common::to_id(*record_type->getDecl());
        }
        else
            // This could be a template parameter - we don't want it here
            continue;

        common::model::relationship cp{parent_id,
            common::access_specifier_to_access_t(base.getAccessSpecifier()),
            base.isVirtual()};

        LOG_DBG("Found base class {} [{}] for class {}",
            common::to_string(base.getType(), cls->getASTContext()),
            parent_id.value(), c.name());

        c.add_relationship(std::move(cp));
    }
}

void translation_unit_visitor::process_record_members(
    const clang::RecordDecl *cls, class_ &c)
{
    // Iterate over regular class fields
    for (const auto *field : cls->fields()) {
        if (field != nullptr)
            process_field(*field, c);
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

    // First we have to collect any `typedef enum` declarations, which should
    // not be rendered as members (typedefs are visited here after enums)
    std::set<const clang::EnumDecl *> typedeffed_enums;
    for (const auto *decl : cls->decls()) {
        if (decl->getKind() == clang::Decl::Typedef) {
            const auto *typedeffed_enum = common::get_typedef_enum_decl(
                clang::dyn_cast<clang::TypedefDecl>(decl));
            if (typedeffed_enum != nullptr)
                typedeffed_enums.emplace(typedeffed_enum);
        }
    }

    // Static fields have to be processed by iterating over variable
    // declarations
    for (const auto *decl : cls->decls()) {
        if (decl->getKind() == clang::Decl::Var) {
            const clang::VarDecl *variable_declaration{
                clang::dyn_cast_or_null<clang::VarDecl>(decl)};
            if ((variable_declaration != nullptr) &&
                variable_declaration->isStaticDataMember()) {
                process_static_field(*variable_declaration, c);
            }
        }
        else if (decl->getKind() == clang::Decl::Enum &&
            typedeffed_enums.count(
                clang::dyn_cast_or_null<clang::EnumDecl>(decl)) == 0) {
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
            if (should_include(friend_type->getAsRecordDecl())) {
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

    auto method_return_type =
        common::to_string(mf.getReturnType(), mf.getASTContext());

    common::ensure_lambda_type_is_relative(config(), method_return_type);

    auto method_name = mf.getNameAsString();
    if (mf.isTemplated()) {
        // Sometimes in template specializations method names contain the
        // template parameters for some reason - drop them
        // Is there a better way to do this?
        method_name = method_name.substr(0, method_name.find('<'));
    }

    class_method method{common::access_specifier_to_access_t(mf.getAccess()),
        util::trim(method_name),
        config().simplify_template_type(method_return_type)};

    method.set_qualified_name(mf.getQualifiedNameAsString());

    process_method_properties(mf, c, method_name, method);

    process_comment(mf, method);

    // Register the source location of the field declaration
    set_source_location(mf, method);

    if (method.skip())
        return;

    for (const auto *param : mf.parameters()) {
        if (param != nullptr)
            process_function_parameter(*param, method, c);
    }

    //  find relationship for return type
    found_relationships_t relationships;

    // Move dereferencing to build() method of template_builder
    if (const auto *templ = mf.getReturnType()
                                .getNonReferenceType()
                                .getUnqualifiedType()
                                ->getAs<clang::TemplateSpecializationType>();
        templ != nullptr) {
        const auto *unaliased_type = templ;
        if (unaliased_type->isTypeAlias())
            unaliased_type = unaliased_type->getAliasedType()
                                 ->getAs<clang::TemplateSpecializationType>();

        if (unaliased_type != nullptr) {
            auto template_specialization_ptr =
                std::make_unique<class_>(config().using_namespace());
            tbuilder().build_from_template_specialization_type(
                *template_specialization_ptr,
                unaliased_type->getTemplateName().getAsTemplateDecl(),
                *unaliased_type, &c);

            template_specialization_ptr->is_template();

            if (diagram().should_include(*template_specialization_ptr)) {
                relationships.emplace_back(template_specialization_ptr->id(),
                    relationship_t::kDependency);

                add_class(std::move(template_specialization_ptr));
            }
        }
    }

    find_relationships(
        mf.getReturnType(), relationships, relationship_t::kDependency);

    for (const auto &[type_element_id, relationship_type] : relationships) {
        if (type_element_id != c.id() &&
            (relationship_type != relationship_t::kNone)) {
            relationship r{relationship_t::kDependency, type_element_id};

            LOG_DBG("Adding method return type relationship from {}::{} to "
                    "{}: {}",
                c, mf.getNameAsString(), r.type(), r.label());

            c.add_relationship(std::move(r));
        }
    }

    // Also consider the container itself if it is a template
    // instantiation it's arguments could count as reference to relevant
    // types
    auto underlying_type = mf.getReturnType();
    if (underlying_type->isReferenceType())
        underlying_type = underlying_type.getNonReferenceType();
    if (underlying_type->isPointerType())
        underlying_type = underlying_type->getPointeeType();

    if (const auto *atsp = underlying_type->getAs<clang::AutoType>();
        atsp != nullptr) {
        process_function_parameter_find_relationships_in_autotype(c, atsp);
    }

    method.update(config().using_namespace());

    if (diagram().should_include(method)) {
        LOG_DBG("Adding method: {}", method.name());

        c.add_method(std::move(method));
    }
}

void translation_unit_visitor::process_objc_method(
    const clang::ObjCMethodDecl &mf, objc_interface &c)
{
    auto method_return_type =
        common::to_string(mf.getReturnType(), mf.getASTContext());

    objc_method method{common::access_specifier_to_access_t(mf.getAccess()),
        util::trim(mf.getNameAsString()), method_return_type};

    method.set_qualified_name(mf.getQualifiedNameAsString());

    process_comment(mf, method);

    // Register the source location of the field declaration
    set_source_location(mf, method);

    if (method.skip())
        return;

    method.is_static(mf.isClassMethod());
    method.is_optional(mf.isOptional());

    for (const auto *param : mf.parameters()) {
        if (param != nullptr)
            process_objc_method_parameter(*param, method, c);
    }

    // find relationship for return type
    found_relationships_t relationships;

    find_relationships(
        mf.getReturnType(), relationships, relationship_t::kDependency);

    for (const auto &[type_element_id, relationship_type] : relationships) {
        if (type_element_id != c.id() &&
            (relationship_type != relationship_t::kNone)) {
            relationship r{relationship_t::kDependency, type_element_id};

            LOG_DBG("Adding method return type relationship from {}::{} to "
                    "{}: {}",
                c, mf.getNameAsString(), r.type(), r.label());

            c.add_relationship(std::move(r));
        }
    }

    // TODO
    if (diagram().should_include(method)) {
        LOG_DBG("Adding ObjC method: {}", method.name());

        c.add_method(std::move(method));
    }
}

void translation_unit_visitor::process_method_properties(
    const clang::CXXMethodDecl &mf, const class_ &c,
    const std::string &method_name, class_method &method) const
{
    const bool is_constructor = c.name() == method_name;
    const bool is_destructor = fmt::format("~{}", c.name()) == method_name;

#if LLVM_VERSION_MAJOR > 17
    method.is_pure_virtual(mf.isPureVirtual());
#else
    method.is_pure_virtual(mf.isPure());
#endif
    method.is_virtual(mf.isVirtual());
    method.is_const(mf.isConst());
    method.is_defaulted(mf.isDefaulted());
    method.is_deleted(mf.isDeleted());
    method.is_static(mf.isStatic());
    method.is_operator(mf.isOverloadedOperator());
    method.is_constexpr(mf.isConstexprSpecified() && !is_constructor);
    method.is_consteval(mf.isConsteval());
    method.is_constructor(is_constructor);
    method.is_destructor(is_destructor);
    method.is_move_assignment(mf.isMoveAssignmentOperator());
    method.is_copy_assignment(mf.isCopyAssignmentOperator());
    method.is_noexcept(isNoexceptExceptionSpec(mf.getExceptionSpecType()));
    method.is_coroutine(common::is_coroutine(mf));
}

void translation_unit_visitor::
    process_function_parameter_find_relationships_in_autotype(
        class_ &c, const clang::AutoType *atsp)
{
    auto desugared_atsp = atsp->getDeducedType();

    if (atsp->isSugared()) {
        const auto *deduced_type =
            atsp->desugar()->getAs<clang::DeducedTemplateSpecializationType>();

        if (deduced_type != nullptr)
            desugared_atsp = deduced_type->getDeducedType();
    }

    if (desugared_atsp.isNull())
        return;

    const auto *deduced_record_type = desugared_atsp->isRecordType()
        ? desugared_atsp->getAs<clang::RecordType>()
        : nullptr;

    if (deduced_record_type != nullptr) {
        if (auto *deduced_auto_decl =
                llvm::dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(
                    deduced_record_type->getDecl());
            deduced_auto_decl != nullptr) {

            const auto diagram_class_count_before_visit =
                diagram().classes().size();

            VisitClassTemplateSpecializationDecl(deduced_auto_decl);

            const bool visitor_added_new_template_specialization =
                (diagram().classes().size() -
                    diagram_class_count_before_visit) > 0;

            if (visitor_added_new_template_specialization) {
                const auto &template_specialization_model =
                    diagram().classes().back();

                if (should_include(deduced_auto_decl)) {
                    relationship r{relationship_t::kDependency,
                        template_specialization_model.get().id()};

                    c.add_relationship(std::move(r));
                }
            }
        }
    }
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

    auto method_name = mf.getNameAsString();
    if (mf.isTemplated()) {
        // Sometimes in template specializations method names contain the
        // template parameters for some reason - drop them
        // Is there a better way to do this?
        method_name = method_name.substr(0, method_name.find('<'));
    }
    util::if_not_null(
        clang::dyn_cast<clang::CXXMethodDecl>(mf.getTemplatedDecl()),
        [&](const auto *decl) {
            process_method_properties(*decl, c, method_name, method);
        });

    tbuilder().build_from_template_declaration(method, mf);

    process_comment(mf, method);

    if (method.skip())
        return;

    for (const auto *param : mf.getTemplatedDecl()->parameters()) {
        if (param != nullptr)
            process_function_parameter(*param, method, c);
    }

    method.update(config().using_namespace());

    if (diagram().should_include(method)) {
        LOG_DBG("Adding method: {}", method.name());

        c.add_method(std::move(method));
    }
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
            // Use AST's local ID here for relationship target, as we can't
            // calculate here properly the ID for nested enums. It will be
            // resolved properly in finalize().
            relationships.emplace_back(
                enum_type->getDecl()->getID(), relationship_hint);
        }
    }
    // TODO: Objc support
    else if (type->isRecordType()) {
        const auto *type_instantiation_decl =
            type->getAs<clang::TemplateSpecializationType>();

        if (type_instantiation_decl != nullptr) {
            // If this template should be included in the diagram
            // add it - and then process recursively its arguments
            if (should_include(type_instantiation_decl->getTemplateName()
                                   .getAsTemplateDecl())) {
                relationships.emplace_back(
                    type_instantiation_decl->getTemplateName()
                        .getAsTemplateDecl()
                        ->getID(),
                    relationship_hint);
            }
            for (const auto &template_argument :
                type_instantiation_decl->template_arguments()) {
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
        else if (type->getAsCXXRecordDecl() != nullptr) {
            relationships.emplace_back(
                type->getAsCXXRecordDecl()->getID(), relationship_hint);
            result = true;
        }
        else {
            relationships.emplace_back(
                type->getAsRecordDecl()->getID(), relationship_hint);
            result = true;
        }
    }
    else if (const auto *template_specialization_type =
                 type->getAs<clang::TemplateSpecializationType>();
             template_specialization_type != nullptr) {
        if (should_include(template_specialization_type->getTemplateName()
                               .getAsTemplateDecl())) {
            relationships.emplace_back(
                template_specialization_type->getTemplateName()
                    .getAsTemplateDecl()
                    ->getID(),
                relationship_hint);
        }
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
                for (const auto &param_type : function_type->param_types()) {
                    result = find_relationships(
                        param_type, relationships, relationship_t::kDependency);
                }
            }
            else if (template_argument_kind ==
                clang::TemplateArgument::ArgKind::Type) {
                result = find_relationships(template_argument.getAsType(),
                    relationships, relationship_hint);
            }
        }
    }

    return result;
}

void translation_unit_visitor::process_objc_method_parameter(
    const clang::ParmVarDecl &param, objc_method &method, objc_interface &c)
{
    method_parameter parameter;
    parameter.set_name(param.getNameAsString());

    process_comment(param, parameter);

    if (parameter.skip())
        return;

    auto parameter_type =
        common::to_string(param.getType(), param.getASTContext());
    parameter.set_type(parameter_type);

    if (!parameter.skip_relationship()) {
        // find relationship for the type
        found_relationships_t relationships;

        LOG_DBG("Looking for relationships in type: {}",
            common::to_string(param.getType(), param.getASTContext()));

        find_relationships(
            param.getType(), relationships, relationship_t::kDependency);

        for (const auto &[type_element_id, relationship_type] : relationships) {
            if (type_element_id != c.id() &&
                (relationship_type != relationship_t::kNone)) {
                relationship r{relationship_t::kDependency, type_element_id};

                LOG_DBG("Adding ObjC method parameter relationship from {} to "
                        "{}: {}",
                    c, r.type(), r.label());

                c.add_relationship(std::move(r));
            }
        }
    }

    method.add_parameter(std::move(parameter));
}

void translation_unit_visitor::process_function_parameter(
    const clang::ParmVarDecl &p, class_method &method, class_ &c,
    const std::set<std::string> & /*template_parameter_names*/)
{
    method_parameter parameter;
    parameter.set_name(p.getNameAsString());

    process_comment(p, parameter);

    if (parameter.skip())
        return;

    auto parameter_type = common::to_string(p.getType(), p.getASTContext());

    // Is there no better way to determine that 'type' is a lambda?
    common::ensure_lambda_type_is_relative(config(), parameter_type);

    parameter.set_type(parameter_type);

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

        LOG_DBG("Looking for relationships in type: {}",
            common::to_string(p.getType(), p.getASTContext()));

        if (const auto *templ =
                p.getType()
                    .getNonReferenceType()
                    .getUnqualifiedType()
                    ->getAs<clang::TemplateSpecializationType>();
            templ != nullptr) {
            auto template_specialization_ptr =
                std::make_unique<class_>(config().using_namespace());
            tbuilder().build_from_template_specialization_type(
                *template_specialization_ptr,
                templ->getTemplateName().getAsTemplateDecl(), *templ, &c);

            template_specialization_ptr->is_template(true);

            if (diagram().should_include(*template_specialization_ptr)) {
                relationships.emplace_back(template_specialization_ptr->id(),
                    relationship_t::kDependency);

                add_class(std::move(template_specialization_ptr));
            }
        }

        find_relationships(
            p.getType(), relationships, relationship_t::kDependency);

        for (const auto &[type_element_id, relationship_type] : relationships) {
            if (type_element_id != c.id() &&
                (relationship_type != relationship_t::kNone)) {
                relationship r{relationship_t::kDependency, type_element_id};
                LOG_DBG("Adding function parameter relationship from {} to "
                        "{}: {}",
                    c, r.type(), r.label());

                c.add_relationship(std::move(r));
            }
        }
    }

    method.add_parameter(std::move(parameter));
}

void translation_unit_visitor::add_relationships(
    common::model::diagram_element &c, const model::class_member_base &field,
    const found_relationships_t &relationships, bool break_on_first_aggregation)
{
    auto [decorator_rtype, decorator_rmult] = field.get_relationship();

    for (const auto &[target, relationship_type] : relationships) {
        if (relationship_type != relationship_t::kNone) {
            relationship r{relationship_type, target};
            r.set_label(field.name());
            r.set_access(field.access());
            bool mulitplicity_provided_in_comment{false};
            if (decorator_rtype != relationship_t::kNone) {
                r.set_type(decorator_rtype);
                auto mult = util::split(decorator_rmult, ":", false);
                if (mult.size() == 2) {
                    mulitplicity_provided_in_comment = true;
                    r.set_multiplicity_source(mult[0]);
                    r.set_multiplicity_destination(mult[1]);
                }
            }
            if (!mulitplicity_provided_in_comment &&
                field.destination_multiplicity().has_value()) {
                r.set_multiplicity_destination(
                    std::to_string(*field.destination_multiplicity()));
            }

            r.set_style(field.style_spec());

            LOG_DBG("Adding relationship from {} to {} with label {}", c,
                r.destination(), r.type(), r.label());

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
        field_declaration.getNameAsString(),
        config().simplify_template_type(type_name)};

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
    auto c_ptr = std::make_unique<class_>(config().using_namespace());
    tbuilder().build_from_class_template_specialization(*c_ptr, *cls);

    auto &template_instantiation = *c_ptr;
    template_instantiation.is_template(true);

    // TODO: refactor to method get_qualified_name()
    auto qualified_name = cls->getQualifiedNameAsString();
    util::replace_all(qualified_name, "(anonymous namespace)", "");
    util::replace_all(qualified_name, "::::", "::");

    namespace_ ns{qualified_name};
    ns.pop_back();
    template_instantiation.set_name(cls->getNameAsString());
    template_instantiation.set_namespace(ns);

    template_instantiation.is_struct(cls->isStruct());

    process_record_parent(cls, template_instantiation, ns);

    if (!template_instantiation.is_nested()) {
        template_instantiation.set_name(common::get_tag_name(*cls));
        template_instantiation.set_id(
            common::to_id(template_instantiation.full_name(false)));
    }

    process_comment(*cls, template_instantiation);
    set_source_location(*cls, template_instantiation);
    set_owning_module(*cls, template_instantiation);

    if (template_instantiation.skip())
        return {};

    id_mapper().add(cls->getID(), template_instantiation.id());

    return c_ptr;
}

void translation_unit_visitor::process_field(
    const clang::FieldDecl &field_declaration, class_ &c)
{
    LOG_DBG(
        "== Visiting record member {}", field_declaration.getNameAsString());

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

    auto field_type_str =
        common::to_string(field_type, field_declaration.getASTContext(), false);

    common::ensure_lambda_type_is_relative(config(), field_type_str);

    class_member field{
        common::access_specifier_to_access_t(field_declaration.getAccess()),
        field_name, config().simplify_template_type(field_type_str)};

    field.set_qualified_name(field_declaration.getQualifiedNameAsString());

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
    else if (field_type->isArrayType()) {
        relationship_hint = relationship_t::kAggregation;
        while (field_type->isArrayType()) {
            auto current_multiplicity = field.destination_multiplicity();
            if (!current_multiplicity)
                field.set_destination_multiplicity(common::get_array_size(
                    *field_type->getAsArrayTypeUnsafe()));
            else {
                auto maybe_array_size =
                    common::get_array_size(*field_type->getAsArrayTypeUnsafe());
                if (maybe_array_size.has_value()) {
                    field.set_destination_multiplicity(
                        current_multiplicity.value() *
                        maybe_array_size.value());
                }
            }

            field_type = field_type->getAsArrayTypeUnsafe()->getElementType();
        }
    }
    else if (field_type->isRValueReferenceType()) {
        field_type = field_type.getNonReferenceType();
    }

    if (type_name.find("std::shared_ptr") == 0)
        relationship_hint = relationship_t::kAssociation;
    if (type_name.find("std::weak_ptr") == 0)
        relationship_hint = relationship_t::kAssociation;

    found_relationships_t relationships;

    const auto *template_field_type =
        field_type->getAs<clang::TemplateSpecializationType>();
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
        for (const auto &class_template_param : c.template_params()) {
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
            std::make_unique<class_>(config().using_namespace());
        tbuilder().build_from_template_specialization_type(
            *template_specialization_ptr,
            field_type->getAs<clang::TemplateSpecializationType>()
                ->getTemplateName()
                .getAsTemplateDecl(),
            *template_field_type, {&c});
        template_specialization_ptr->is_template(true);

        if (!field.skip_relationship() && template_specialization_ptr) {
            const auto &template_specialization = *template_specialization_ptr;

            // Check if this template instantiation should be added to the
            // current diagram. Even if the top level template type for
            // this instantiation should not be part of the diagram, e.g.
            // it's a std::vector<>, it's nested types might be added
            bool add_template_instantiation_to_diagram{false};
            if (diagram().should_include(
                    template_specialization.get_namespace())) {

                found_relationships_t::value_type r{
                    template_specialization.id(), relationship_hint};

                add_template_instantiation_to_diagram = true;

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
                    template_specialization.template_params()) {

                    LOG_DBG("Looking for nested relationships from {}::{} in "
                            "template argument {}",
                        c, field_name,
                        template_argument.to_string(
                            config().using_namespace(), false));

                    template_instantiation_added_as_aggregation =
                        template_argument.find_nested_relationships(
                            nested_relationships, relationship_hint,
                            [&d = diagram()](const std::string &full_name) {
                                if (full_name.empty())
                                    return false;
                                auto [ns, name] = common::split_ns(full_name);
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
            if (add_template_instantiation_to_diagram)
                add_class(std::move(template_specialization_ptr));
        }
    }

    if (!field.skip_relationship()) {
        // Find relationship for the type if the type has not been added
        // as aggregation
        if (!template_instantiation_added_as_aggregation) {
            if ((field_type->getAsRecordDecl() != nullptr) &&
                field_type->getAsRecordDecl()->getNameAsString().empty()) {
                // Relationships to fields whose type is an anonymous nested
                // struct have to be handled separately here
                anonymous_struct_relationships_[field_type->getAsRecordDecl()
                                                    ->getID()] =
                    std::make_tuple(field.name(), relationship_hint,
                        field.access(), field.destination_multiplicity());
            }
            else
                find_relationships(
                    field_type, relationships, relationship_hint);
        }

        add_relationships(c, field, relationships);
    }

    // If this is an anonymous struct - replace the anonymous_XYZ part with
    // field name
    if ((field_type->getAsRecordDecl() != nullptr) &&
        field_type->getAsRecordDecl()->getNameAsString().empty()) {
        if (util::contains(field.type(), "(anonymous_")) {
            std::regex anonymous_re("anonymous_(\\d*)");
            field.set_type(
                std::regex_replace(field.type(), anonymous_re, field_name));
        }
    }

    if (diagram().should_include(field)) {
        c.add_member(std::move(field));
    }
}

void translation_unit_visitor::find_record_parent_id(const clang::TagDecl *decl,
    std::optional<eid_t> &parent_id_opt, namespace_ &parent_ns) const
{
    const auto *parent = decl->getParent();

    if (parent != nullptr) {
        if (const auto *parent_record_decl =
                clang::dyn_cast<clang::RecordDecl>(parent);
            parent_record_decl != nullptr) {
            parent_ns = common::get_tag_namespace(*parent_record_decl);

            eid_t local_id{parent_record_decl->getID()};

            // First check if the parent has been added to the diagram as
            // regular class
            parent_id_opt = id_mapper().get_global_id(local_id);

            // If not, check if the parent template declaration is in the model
            if (!parent_id_opt) {
                if (parent_record_decl->getDescribedTemplate() != nullptr) {
                    local_id =
                        parent_record_decl->getDescribedTemplate()->getID();
                    parent_id_opt = id_mapper().get_global_id(local_id);
                }
            }
        }
    }

    if (parent_id_opt)
        return;

    const auto *lexical_parent = decl->getLexicalParent();
    if (lexical_parent != nullptr) {
        if (const auto *parent_interface_decl =
                clang::dyn_cast<clang::ObjCInterfaceDecl>(lexical_parent);
            parent_interface_decl != nullptr) {

            eid_t ast_id{parent_interface_decl->getID()};

            // First check if the parent has been added to the diagram as
            // regular class
            parent_id_opt = id_mapper().get_global_id(ast_id);
        }
    }
}

void translation_unit_visitor::add_incomplete_forward_declarations()
{
    for (auto &[id, c] : forward_declarations_) {
        if (diagram().should_include(c->get_namespace())) {
            add_class(std::move(c));
        }
    }
    forward_declarations_.clear();
}

void translation_unit_visitor::resolve_local_to_global_ids()
{
    diagram().for_all_elements([&](auto &element_view) {
        for (const auto &el : element_view) {
            for (auto &rel : el.get().relationships()) {
                if (!rel.destination().is_global()) {
                    const auto maybe_id =
                        id_mapper().get_global_id(rel.destination());
                    if (maybe_id) {
                        LOG_TRACE(
                            "= Resolved instantiation destination from local "
                            "id {} to global id {}",
                            rel.destination(), *maybe_id);
                        rel.set_destination(*maybe_id);
                    }
                }
            }
            el.get().remove_duplicate_relationships();
        }
    });
}

void translation_unit_visitor::finalize()
{
    add_incomplete_forward_declarations();
    resolve_local_to_global_ids();
    if (config().skip_redundant_dependencies()) {
        diagram().remove_redundant_dependencies();
    }
}

void translation_unit_visitor::extract_constrained_template_param_name(
    const clang::ConceptSpecializationExpr *concept_specialization,
    const clang::ConceptDecl *cpt,
    std::vector<std::string> &constrained_template_params,
    size_t argument_index, std::string &type_name) const
{
    const auto full_declaration_text = common::get_source_text_raw(
        concept_specialization->getSourceRange(), source_manager());

    if (!full_declaration_text.empty()) {
        // Handle typename constraint in requires clause
        if (type_name.find("type-parameter-") == 0) {
            const auto concept_declaration_text = full_declaration_text.substr(
                full_declaration_text.find(cpt->getNameAsString()) +
                cpt->getNameAsString().size() + 1);

            auto template_params = common::parse_unexposed_template_params(
                concept_declaration_text, [](const auto &t) { return t; });

            if (template_params.size() > argument_index)
                type_name = template_params[argument_index].to_string(
                    config().using_namespace(), false);
        }
        constrained_template_params.push_back(type_name);
    }
}

void translation_unit_visitor::add_processed_template_class(
    std::string qualified_name)
{
    processed_template_qualified_names_.emplace(std::move(qualified_name));
}

bool translation_unit_visitor::has_processed_template_class(
    const std::string &qualified_name) const
{
    return util::contains(processed_template_qualified_names_, qualified_name);
}

void translation_unit_visitor::add_diagram_element(
    std::unique_ptr<common::model::template_element> element)
{
    add_class(util::unique_pointer_cast<class_>(std::move(element)));
}

void translation_unit_visitor::add_class(std::unique_ptr<class_> &&c)
{
    c->complete(true);

    if ((config().generate_packages() &&
            config().package_type() == config::package_type_t::kDirectory)) {
        assert(!c->file().empty());

        const auto file = config().make_path_relative(c->file());

        common::model::path p{
            file.string(), common::model::path_type::kFilesystem};
        p.pop_back();

        diagram().add(p, std::move(c));
    }
    else if ((config().generate_packages() &&
                 config().package_type() == config::package_type_t::kModule)) {

        const auto module_path = config().make_module_relative(c->module());

        common::model::path p{module_path, common::model::path_type::kModule};

        diagram().add(p, std::move(c));
    }
    else {
        diagram().add(c->path(), std::move(c));
    }
}

void translation_unit_visitor::add_objc_interface(
    std::unique_ptr<objc_interface> &&c)
{
    c->complete(true);

    if ((config().generate_packages() &&
            config().package_type() == config::package_type_t::kDirectory)) {
        assert(!c->file().empty());

        const auto file = config().make_path_relative(c->file());

        common::model::path p{
            file.string(), common::model::path_type::kFilesystem};
        p.pop_back();

        diagram().add(p, std::move(c));
    }
    else {
        diagram().add(c->path(), std::move(c));
    }
}

void translation_unit_visitor::add_enum(std::unique_ptr<enum_> &&e)
{
    e->complete(true);

    if ((config().generate_packages() &&
            config().package_type() == config::package_type_t::kDirectory)) {
        assert(!e->file().empty());

        const auto file = config().make_path_relative(e->file());

        common::model::path p{
            file.string(), common::model::path_type::kFilesystem};
        p.pop_back();

        diagram().add(p, std::move(e));
    }
    else if ((config().generate_packages() &&
                 config().package_type() == config::package_type_t::kModule)) {

        const auto module_path = config().make_module_relative(e->module());

        common::model::path p{module_path, common::model::path_type::kModule};

        diagram().add(p, std::move(e));
    }
    else {
        diagram().add(e->path(), std::move(e));
    }
}

void translation_unit_visitor::add_concept(std::unique_ptr<concept_> &&c)
{
    c->complete(true);

    if ((config().generate_packages() &&
            config().package_type() == config::package_type_t::kDirectory)) {
        assert(!c->file().empty());

        const auto file = config().make_path_relative(c->file());

        common::model::path p{
            file.string(), common::model::path_type::kFilesystem};
        p.pop_back();

        diagram().add(p, std::move(c));
    }
    else if ((config().generate_packages() &&
                 config().package_type() == config::package_type_t::kModule)) {

        const auto module_path = config().make_module_relative(c->module());

        common::model::path p{module_path, common::model::path_type::kModule};

        diagram().add(p, std::move(c));
    }
    else {
        diagram().add(c->path(), std::move(c));
    }
}

void translation_unit_visitor::find_instantiation_relationships(
    common::model::template_element &template_instantiation_base,
    const std::string &full_name, eid_t templated_decl_id)
{
    auto &template_instantiation = dynamic_cast<class_diagram::model::class_ &>(
        template_instantiation_base);

    // First try to find the best match for this template in partially
    // specialized templates
    std::string destination{};
    std::string best_match_full_name{};
    auto full_template_name = template_instantiation.full_name(false);
    int best_match{};
    eid_t best_match_id{};

    for (const auto templ : diagram().classes()) {
        if (templ.get() == template_instantiation)
            continue;

        auto c_full_name = templ.get().full_name(false);
        auto match =
            template_instantiation.calculate_template_specialization_match(
                templ.get());

        if (match > best_match) {
            best_match = match;
            best_match_full_name = c_full_name;
            best_match_id = templ.get().id();
        }
    }

    auto templated_decl_global_id =
        id_mapper().get_global_id(templated_decl_id).value_or(eid_t{});

    if (best_match_id.value() > 0) {
        destination = best_match_full_name;
        template_instantiation.add_relationship(
            {common::model::relationship_t::kInstantiation, best_match_id});
        template_instantiation.template_specialization_found(true);
    }
    // If we can't find optimal match for parent template specialization,
    // just use whatever clang suggests
    else if (diagram().has_element(templated_decl_global_id)) {
        template_instantiation.add_relationship(
            {common::model::relationship_t::kInstantiation,
                templated_decl_global_id});
        template_instantiation.template_specialization_found(true);
    }
    else if (diagram().should_include(common::model::namespace_{full_name})) {
        LOG_DBG("Skipping instantiation relationship from {} to {}",
            template_instantiation, templated_decl_global_id);
    }
    else {
        LOG_DBG("== Cannot determine global id for specialization template {} "
                "- delaying until the translation unit is complete ",
            templated_decl_global_id);

        template_instantiation.add_relationship(
            {common::model::relationship_t::kInstantiation, templated_decl_id});
    }
}

} // namespace clanguml::class_diagram::visitor
