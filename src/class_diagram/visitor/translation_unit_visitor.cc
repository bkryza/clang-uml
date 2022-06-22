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

#include "cppast/cpp_function_type.hpp"
#include "cx/util.h"

#include <cppast/cpp_alias_template.hpp>
#include <cppast/cpp_array_type.hpp>
#include <cppast/cpp_class_template.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_friend.hpp>
#include <cppast/cpp_function_type.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <cppast/cpp_namespace.hpp>
#include <cppast/cpp_template.hpp>
#include <cppast/cpp_type_alias.hpp>
#include <cppast/cpp_variable.hpp>
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
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;

namespace detail {
access_t cpp_access_specifier_to_access(
    cppast::cpp_access_specifier_kind access_specifier)
{
    auto access = access_t::kPublic;
    switch (access_specifier) {
    case cppast::cpp_access_specifier_kind::cpp_public:
        access = access_t::kPublic;
        break;
    case cppast::cpp_access_specifier_kind::cpp_private:
        access = access_t::kPrivate;
        break;
    case cppast::cpp_access_specifier_kind::cpp_protected:
        access = access_t::kProtected;
        break;
    default:
        break;
    }

    return access;
}
}

translation_unit_visitor::translation_unit_visitor(
    cppast::cpp_entity_index &idx,
    clanguml::class_diagram::model::diagram &diagram,
    const clanguml::config::class_diagram &config)
    : ctx{idx, diagram, config}
{
}

void translation_unit_visitor::operator()(const cppast::cpp_entity &file)
{
    cppast::visit(file,
        [&, this](const cppast::cpp_entity &e, cppast::visitor_info info) {
            if (e.kind() == cppast::cpp_entity_kind::namespace_t) {
                if (info.event ==
                    cppast::visitor_info::container_entity_enter) {
                    LOG_DBG("========== Visiting '{}' - {}", e.name(),
                        cppast::to_string(e.kind()));

                    const auto &ns_declaration =
                        static_cast<const cppast::cpp_namespace &>(e);
                    if (!ns_declaration.is_anonymous() &&
                        !ns_declaration.is_inline()) {

                        process_namespace(e, ns_declaration);
                    }
                }
                else {
                    LOG_DBG("========== Leaving '{}' - {}", e.name(),
                        cppast::to_string(e.kind()));

                    const auto &ns_declaration =
                        static_cast<const cppast::cpp_namespace &>(e);
                    if (!ns_declaration.is_anonymous() &&
                        !ns_declaration.is_inline())
                        ctx.pop_namespace();
                }
            }
            else if (e.kind() == cppast::cpp_entity_kind::namespace_alias_t) {
                auto &na = static_cast<const cppast::cpp_namespace_alias &>(e);

                for (const auto &alias_target :
                    na.target().get(ctx.entity_index())) {
                    auto full_ns = cx::util::full_name(ctx.get_namespace(), na);
                    ctx.add_namespace_alias(full_ns, alias_target);
                }
            }
            else if (e.kind() ==
                cppast::cpp_entity_kind::class_template_specialization_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.get_namespace(), e),
                    cppast::to_string(e.kind()));

                auto &tspec = static_cast<
                    const cppast::cpp_class_template_specialization &>(e);

                process_class_declaration(
                    tspec.class_(), type_safe::ref(tspec));
            }
            else if (e.kind() == cppast::cpp_entity_kind::class_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.get_namespace(), e),
                    cppast::to_string(e.kind()));

                auto &cls = static_cast<const cppast::cpp_class &>(e);
                if (cppast::get_definition(ctx.entity_index(), cls)) {
                    const auto &clsdef = static_cast<const cppast::cpp_class &>(
                        cppast::get_definition(ctx.entity_index(), cls)
                            .value());
                    if (&cls != &clsdef) {
                        LOG_DBG("Forward declaration of class {} - skipping...",
                            cls.name());
                        return;
                    }
                }

                if (ctx.diagram().should_include(
                        ctx.get_namespace(), cls.name()))
                    process_class_declaration(cls);
            }
            else if (e.kind() == cppast::cpp_entity_kind::enum_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.get_namespace(), e),
                    cppast::to_string(e.kind()));

                auto &enm = static_cast<const cppast::cpp_enum &>(e);

                if (ctx.diagram().should_include(
                        ctx.get_namespace(), enm.name()))
                    process_enum_declaration(enm);
            }
            else if (e.kind() == cppast::cpp_entity_kind::type_alias_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.get_namespace(), e),
                    cppast::to_string(e.kind()));

                auto &ta = static_cast<const cppast::cpp_type_alias &>(e);
                process_type_alias(ta);
            }
            else if (e.kind() == cppast::cpp_entity_kind::alias_template_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.get_namespace(), e),
                    cppast::to_string(e.kind()));

                auto &at = static_cast<const cppast::cpp_alias_template &>(e);

                process_type_alias_template(at);
            }
            else if (e.kind() == cppast::cpp_entity_kind::using_directive_t) {
                using common::model::namespace_;

                const auto &using_directive =
                    static_cast<const cppast::cpp_using_directive &>(e);

                const auto ns_ref = using_directive.target();
                const auto &ns = ns_ref.get(ctx.entity_index()).at(0).get();
                if (ns_ref.get(ctx.entity_index()).size() > 0) {
                    auto full_ns = namespace_{cx::util::ns(ns)} | ns.name();

                    ctx.add_using_namespace_directive(full_ns);
                }
            }
        });
}

void translation_unit_visitor::process_type_alias_template(
    const cppast::cpp_alias_template &at)
{
    auto alias_kind = at.type_alias().underlying_type().kind();

    if (alias_kind == cppast::cpp_type_kind::unexposed_t) {
        LOG_DBG("Template alias has unexposed underlying type - ignoring: {}",
            static_cast<const cppast::cpp_unexposed_type &>(
                at.type_alias().underlying_type())
                .name());
    }
    else {
        if (at.type_alias().underlying_type().kind() ==
            cppast::cpp_type_kind::template_instantiation_t) {
            auto tinst = build_template_instantiation(
                static_cast<const cppast::cpp_template_instantiation_type &>(
                    resolve_alias(at.type_alias().underlying_type())));

            assert(tinst);

            tinst->is_alias(true);

            if (tinst->get_namespace().is_empty())
                tinst->set_namespace(ctx.get_namespace());

            ctx.add_type_alias_template(
                cx::util::full_name(ctx.get_namespace(), at),
                type_safe::ref(at.type_alias().underlying_type()));

            if (ctx.diagram().should_include(
                    tinst->get_namespace(), tinst->name()))
                ctx.diagram().add_class(std::move(tinst));
        }
        else {
            LOG_DBG("Unsupported alias target...");
        }
    }
}

void translation_unit_visitor::process_type_alias(
    const cppast::cpp_type_alias &ta)
{
    auto t = std::make_unique<type_alias>();
    t->set_alias(cx::util::full_name(ctx.get_namespace(), ta));
    t->set_underlying_type(cx::util::full_name(ta.underlying_type(),
        ctx.entity_index(), cx::util::is_inside_class(ta)));

    ctx.add_type_alias(cx::util::full_name(ctx.get_namespace(), ta),
        type_safe::ref(ta.underlying_type()));

    ctx.diagram().add_type_alias(std::move(t));
}

void translation_unit_visitor::process_namespace(
    const cppast::cpp_entity &e, const cppast::cpp_namespace &ns_declaration)
{
    auto package_parent = ctx.get_namespace();
    auto package_path = package_parent | e.name();

    auto usn = ctx.config().using_namespace();

    auto p = std::make_unique<common::model::package>(usn);
    package_path = package_path.relative_to(usn);

    p->set_name(e.name());
    p->set_namespace(package_parent);

    if (ctx.diagram().should_include(*p)) {
        if (e.comment().has_value())
            p->set_comment(e.comment().value());

        if (e.location().has_value()) {
            p->set_file(e.location().value().file);
            p->set_line(e.location().value().line);
        }

        if (ns_declaration.comment().has_value())
            p->add_decorators(
                decorators::parse(ns_declaration.comment().value()));

        p->set_style(p->style_spec());

        for (const auto &attr : ns_declaration.attributes()) {
            if (attr.kind() == cppast::cpp_attribute_kind::deprecated) {
                p->set_deprecated(true);
                break;
            }
        }

        if (!p->skip()) {
            ctx.diagram().add_package(std::move(p));
            ctx.set_current_package(
                ctx.diagram().get_element<common::model::package>(
                    package_path));
        }
    }
    ctx.push_namespace(e.name());
}

void translation_unit_visitor::process_enum_declaration(
    const cppast::cpp_enum &enm)
{
    if (enm.name().empty()) {
        // Anonymous enum values should be rendered as class fields
        // with type enum
        return;
    }

    auto e_ptr = std::make_unique<enum_>(ctx.config().using_namespace());
    auto &e = *e_ptr;
    e.set_name(enm.name());
    e.set_namespace(ctx.get_namespace());

    if (enm.comment().has_value())
        e.set_comment(enm.comment().value());

    if (enm.location().has_value()) {
        e.set_file(enm.location().value().file);
        e.set_line(enm.location().value().line);
    }

    if (enm.comment().has_value())
        e.add_decorators(decorators::parse(enm.comment().value()));

    if (e.skip())
        return;

    e.set_style(e.style_spec());

    // Process enum documentation comment
    if (enm.comment().has_value())
        e.add_decorators(decorators::parse(enm.comment().value()));

    for (const auto &ev : enm) {
        if (ev.kind() == cppast::cpp_entity_kind::enum_value_t) {
            e.constants().push_back(ev.name());
        }
    }

    // Find if enum is contained in a class
    for (auto cur = enm.parent(); cur; cur = cur.value().parent()) {
        // find nearest parent class, if any
        if (cur.value().kind() == cppast::cpp_entity_kind::class_t) {
            e.add_relationship({relationship_t::kContainment,
                cx::util::full_name(ctx.get_namespace(), cur.value())});

            LOG_DBG("Added containment relationship {} +-- {}",
                cx::util::full_name(ctx.get_namespace(), cur.value()),
                e.name());
            break;
        }
    }

    ctx.diagram().add_enum(std::move(e_ptr));
}

void translation_unit_visitor::process_class_declaration(
    const cppast::cpp_class &cls,
    type_safe::optional_ref<const cppast::cpp_template_specialization> tspec)
{
    auto c_ptr = std::make_unique<class_>(ctx.config().using_namespace());
    auto &c = *c_ptr;

    if (cls.location().has_value()) {
        c.set_file(cls.location().value().file);
        c.set_line(cls.location().value().line);
    }

    c.is_struct(cls.class_kind() == cppast::cpp_class_kind::struct_t);

    c.set_name(cls.name());
    c.set_namespace(ctx.get_namespace());

    if (cls.comment().has_value()) {
        c.set_comment(cls.comment().value());
        c.add_decorators(decorators::parse(cls.comment().value()));
    }

    // Process class documentation comment
    if (cppast::is_templated(cls)) {
        if (cls.parent().value().comment().has_value())
            c.add_decorators(
                decorators::parse(cls.parent().value().comment().value()));
    }
    else {
        if (cls.comment().has_value())
            c.add_decorators(decorators::parse(cls.comment().value()));
    }

    if (c.skip())
        return;

    c.set_style(c.style_spec());

    // Process class child entities
    process_class_children(cls, c);

    // Process class bases
    process_class_bases(cls, c);

    // Process class template arguments
    if (cppast::is_templated(cls)) {
        bool skip = process_template_parameters(cls, c, tspec);
        if (skip)
            return;
    }

    // Find if class is contained in another class
    process_class_containment(cls, c);

    cls.set_user_data(strdup(c.full_name().c_str()));

    LOG_DBG("Setting user data for class {}, {}",
        static_cast<const char *>(cls.user_data()),
        fmt::ptr(reinterpret_cast<const void *>(&cls)));

    assert(c_ptr);

    if (ctx.diagram().should_include(c))
        ctx.diagram().add_class(std::move(c_ptr));
}

void translation_unit_visitor::process_class_containment(
    const cppast::cpp_class &cls, class_ &c) const
{
    for (auto cur = cls.parent(); cur; cur = cur.value().parent()) {
        // find nearest parent class, if any
        if (cur.value().kind() == cppast::cpp_entity_kind::class_t) {
            c.add_relationship({relationship_t::kContainment,
                cx::util::full_name(ctx.get_namespace(), cur.value())});

            LOG_DBG("Added containment relationship {}", c.full_name());

            break;
        }
    }
}

bool translation_unit_visitor::process_template_parameters(
    const cppast::cpp_class &cls, class_ &c,
    const type_safe::optional_ref<const cppast::cpp_template_specialization>
        &tspec)
{
    LOG_DBG("Processing class {} template parameters...", cls.name());

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

    return false;
}

void translation_unit_visitor::process_scope_template_parameters(
    class_ &c, const cppast::cpp_scope_name &scope)
{
    for (const auto &tp : scope.template_parameters()) {
        if (tp.kind() == cppast::cpp_entity_kind::template_type_parameter_t) {
            LOG_DBG("Processing template type parameter {}", tp.name());
            process_template_type_parameter(
                static_cast<const cppast::cpp_template_type_parameter &>(tp),
                c);
        }
        else if (tp.kind() ==
            cppast::cpp_entity_kind::non_type_template_parameter_t) {
            LOG_DBG("Processing template nontype parameter {}", tp.name());
            process_template_nontype_parameter(
                static_cast<const cppast::cpp_non_type_template_parameter &>(
                    tp),
                c);
        }
        else if (tp.kind() ==
            cppast::cpp_entity_kind::template_template_parameter_t) {
            LOG_DBG("Processing template template parameter {}", tp.name());
            process_template_template_parameter(
                static_cast<const cppast::cpp_template_template_parameter &>(
                    tp),
                c);
        }
    }
}

void translation_unit_visitor::
    process_exposed_template_specialization_parameters(
        const type_safe::optional_ref<const cppast::cpp_template_specialization>
            &tspec,
        class_ &c)
{
    for (auto &tp : tspec.value().parameters()) {
        switch (tp.kind()) {
        case cppast::cpp_entity_kind::template_type_parameter_t: {
            LOG_DBG("Processing template type parameter {}", tp.name());
            process_template_type_parameter(
                static_cast<const cppast::cpp_template_type_parameter &>(tp),
                c);
        } break;
        case cppast::cpp_entity_kind::non_type_template_parameter_t: {
            LOG_DBG("Processing template nontype parameter {}", tp.name());
            process_template_nontype_parameter(
                static_cast<const cppast::cpp_non_type_template_parameter &>(
                    tp),
                c);
        } break;
        case cppast::cpp_entity_kind::template_template_parameter_t: {
            LOG_DBG("Processing template template parameter {}", tp.name());
            process_template_template_parameter(
                static_cast<const cppast::cpp_template_template_parameter &>(
                    tp),
                c);
        } break;
        default:
            LOG_DBG("Unhandled template parameter "
                    "type {}",
                cppast::to_string(tp.kind()));
            break;
        }
    }
}

void translation_unit_visitor::
    process_unexposed_template_specialization_parameters(
        const type_safe::optional_ref<const cppast::cpp_template_specialization>
            &tspec,
        class_ &c) const
{
    auto ua = tspec.value().unexposed_arguments().as_string();

    auto template_params = cx::util::parse_unexposed_template_params(
        ua, [this](const std::string &t) {
            auto full_type = ctx.get_name_with_namespace(t);
            if (full_type.has_value())
                return full_type.value().to_string();
            return t;
        });

    found_relationships_t relationships;
    for (auto &param : template_params) {
        find_relationships_in_unexposed_template_params(param, relationships);
        c.add_template(param);
    }

    for (auto &r : relationships) {
        c.add_relationship({std::get<1>(r), std::get<0>(r)});
    }

    if (!tspec.has_value() ||
        tspec.value().primary_template().get(ctx.entity_index()).size() == 0)
        return;

    const auto &primary_template_ref =
        static_cast<const cppast::cpp_class_template &>(
            tspec.value().primary_template().get(ctx.entity_index())[0].get())
            .class_();

    if (primary_template_ref.user_data()) {
        auto base_template_full_name =
            static_cast<const char *>(primary_template_ref.user_data());
        LOG_DBG("Primary template ref set to: {}", base_template_full_name);
        // Add template specialization/instantiation
        // relationship
        c.add_relationship(
            {relationship_t::kInstantiation, base_template_full_name});
    }
    else {
        LOG_DBG(
            "No user data for base template {}", primary_template_ref.name());
    }
}

void translation_unit_visitor::process_class_bases(
    const cppast::cpp_class &cls, class_ &c) const
{
    for (auto &base : cls.bases()) {
        class_parent cp;
        auto ns = cx::util::ns(base.type(), ctx.entity_index());
        common::model::namespace_ base_ns;
        if (!ns.empty())
            base_ns = common::model::namespace_{ns};
        base_ns = base_ns | common::model::namespace_{base.name()}.name();
        cp.set_name(base_ns.to_string());
        cp.is_virtual(base.is_virtual());

        switch (base.access_specifier()) {
        case cppast::cpp_private:
            cp.set_access(access_t::kPrivate);
            break;
        case cppast::cpp_public:
            cp.set_access(access_t::kPublic);
            break;
        case cppast::cpp_protected:
            cp.set_access(access_t::kProtected);
            break;
        default:
            cp.set_access(access_t::kPublic);
        }

        LOG_DBG("Found base class {} for class {}", cp.name(), c.name());

        c.add_parent(std::move(cp));
    }
}

void translation_unit_visitor::process_class_children(
    const cppast::cpp_class &cls, class_ &c)
{
    cppast::cpp_access_specifier_kind last_access_specifier =
        cppast::cpp_access_specifier_kind::cpp_private;

    if (c.is_struct())
        last_access_specifier = cppast::cpp_access_specifier_kind::cpp_public;

    for (auto &child : cls) {
        if (child.kind() == cppast::cpp_entity_kind::access_specifier_t) {
            auto &as = static_cast<const cppast::cpp_access_specifier &>(child);
            last_access_specifier = as.access_specifier();
        }
        else if (child.kind() == cppast::cpp_entity_kind::member_variable_t) {
            auto &mv = static_cast<const cppast::cpp_member_variable &>(child);
            process_field(mv, c, last_access_specifier);
        }
        else if (child.kind() == cppast::cpp_entity_kind::variable_t) {
            auto &mv = static_cast<const cppast::cpp_variable &>(child);
            process_static_field(mv, c, last_access_specifier);
        }
        else if (child.kind() == cppast::cpp_entity_kind::member_function_t) {
            auto &mf = static_cast<const cppast::cpp_member_function &>(child);
            process_method(mf, c, last_access_specifier);
        }
        else if (child.kind() == cppast::cpp_entity_kind::function_t) {
            auto &mf = static_cast<const cppast::cpp_function &>(child);
            process_static_method(mf, c, last_access_specifier);
        }
        else if (child.kind() == cppast::cpp_entity_kind::function_template_t) {
            auto &tm =
                static_cast<const cppast::cpp_function_template &>(child);
            process_template_method(tm, c, last_access_specifier);
        }
        else if (child.kind() == cppast::cpp_entity_kind::constructor_t) {
            auto &mc = static_cast<const cppast::cpp_constructor &>(child);
            process_constructor(mc, c, last_access_specifier);
        }
        else if (child.kind() == cppast::cpp_entity_kind::destructor_t) {
            auto &mc = static_cast<const cppast::cpp_destructor &>(child);
            process_destructor(mc, c, last_access_specifier);
        }
        else if (child.kind() == cppast::cpp_entity_kind::enum_t) {
            auto &en = static_cast<const cppast::cpp_enum &>(child);
            if (en.name().empty()) {
                // Here we only want to handle anonymous enums, regular nested
                // enums are handled in the file-level visitor
                process_anonymous_enum(en, c, last_access_specifier);
            }
        }
        else if (child.kind() == cppast::cpp_entity_kind::friend_t) {
            auto &fr = static_cast<const cppast::cpp_friend &>(child);

            LOG_DBG("Found friend declaration: {}, {}", child.name(),
                child.scope_name() ? child.scope_name().value().name()
                                   : "<no-scope>");

            process_friend(fr, c, last_access_specifier);
        }
        else if (cppast::is_friended(child)) {
            auto &fr =
                static_cast<const cppast::cpp_friend &>(child.parent().value());

            LOG_DBG("Found friend template: {}", child.name());

            process_friend(fr, c, last_access_specifier);
        }
        else {
            LOG_DBG("Found some other class child: {} ({})", child.name(),
                cppast::to_string(child.kind()));
        }
    }
}

bool translation_unit_visitor::process_field_with_template_instantiation(
    const cppast::cpp_member_variable &mv, const cppast::cpp_type &type,
    class_ &c, class_member &member, cppast::cpp_access_specifier_kind as)
{
    LOG_DBG("Processing field with template instantiation type {}",
        cppast::to_string(type));

    bool res = false;

    auto tr_declaration = cppast::to_string(type);

    const auto &template_instantiation_type =
        static_cast<const cppast::cpp_template_instantiation_type &>(type);

    const auto &unaliased =
        static_cast<const cppast::cpp_template_instantiation_type &>(
            resolve_alias(template_instantiation_type));

    auto tr_unaliased_declaration = cppast::to_string(unaliased);

    std::unique_ptr<class_> tinst_ptr;

    found_relationships_t nested_relationships;
    if (tr_declaration == tr_unaliased_declaration)
        tinst_ptr = build_template_instantiation(unaliased, {&c});
    else
        tinst_ptr = build_template_instantiation(
            static_cast<const cppast::cpp_template_instantiation_type &>(
                type.canonical()),
            {&c});

    auto &tinst = *tinst_ptr;

    //
    // Infer the relationship of this field to the template
    // instantiation
    // TODO: Refactor this to a configurable mapping
    relationship_t nested_relationship_hint = relationship_t::kAggregation;

    if (tr_unaliased_declaration.find("std::shared_ptr") == 0) {
        nested_relationship_hint = relationship_t::kAssociation;
    }
    else if (tr_unaliased_declaration.find("std::weak_ptr") == 0) {
        nested_relationship_hint = relationship_t::kAssociation;
    }

    relationship_t relationship_type{};
    if (mv.type().kind() == cppast::cpp_type_kind::pointer_t ||
        mv.type().kind() == cppast::cpp_type_kind::reference_t)
        relationship_type = relationship_t::kAssociation;
    else
        relationship_type = nested_relationship_hint;

    relationship rr{relationship_type, tinst.full_name(),
        detail::cpp_access_specifier_to_access(as), mv.name()};
    rr.set_style(member.style_spec());

    // Process field decorators
    auto [decorator_rtype, decorator_rmult] = member.get_relationship();
    if (decorator_rtype != relationship_t::kNone) {
        rr.set_type(decorator_rtype);
        auto mult = util::split(decorator_rmult, ":", false);
        if (mult.size() == 2) {
            rr.set_multiplicity_source(mult[0]);
            rr.set_multiplicity_destination(mult[1]);
        }
    }

    const auto tinst_namespace = tinst.get_namespace();
    const auto tinst_name = tinst.name();

    // Add instantiation relationship from the generated template instantiation
    // of the field type to its primary template
    if (ctx.diagram().should_include(tinst_namespace, tinst_name)) {
        LOG_DBG("Adding field instantiation relationship {} {} {} : {}",
            rr.destination(), clanguml::common::model::to_string(rr.type()),
            c.full_name(), rr.label());

        c.add_relationship(std::move(rr));

        res = true;

        LOG_DBG("Created template instantiation: {}", tinst.full_name());

        assert(tinst_ptr);

        ctx.diagram().add_class(std::move(tinst_ptr));
    }

    //
    // Only add nested template relationships to this class if the top level
    // template is not in the diagram (e.g. it is a std::shared_ptr<>)
    //
    if (!ctx.diagram().should_include(tinst_namespace, tinst_name)) {
        res = add_nested_template_relationships(mv, c, member, as, tinst,
            relationship_type, decorator_rtype, decorator_rmult);
    }

    return res;
}

bool translation_unit_visitor::add_nested_template_relationships(
    const cppast::cpp_member_variable &mv, class_ &c, class_member &m,
    cppast::cpp_access_specifier_kind &as, const class_ &tinst,
    relationship_t &relationship_type, relationship_t &decorator_rtype,
    std::string &decorator_rmult)
{
    bool res{false};
    found_relationships_t nested_relationships;

    for (const auto &template_argument : tinst.templates()) {
        template_argument.find_nested_relationships(nested_relationships,
            relationship_type,
            [&d = ctx.diagram()](const std::string &full_name) {
                if (full_name.empty())
                    return false;
                auto [ns, name] = cx::util::split_ns(full_name);
                return d.should_include(ns, name);
            });
    }

    if (!nested_relationships.empty()) {
        for (const auto &rel : nested_relationships) {
            relationship nested_relationship{std::get<1>(rel), std::get<0>(rel),
                detail::cpp_access_specifier_to_access(as), mv.name()};
            nested_relationship.set_style(m.style_spec());
            if (decorator_rtype != relationship_t::kNone) {
                nested_relationship.set_type(decorator_rtype);
                auto mult = util::split(decorator_rmult, ":", false);
                if (mult.size() == 2) {
                    nested_relationship.set_multiplicity_source(mult[0]);
                    nested_relationship.set_multiplicity_destination(mult[1]);
                }
            }
            c.add_relationship(std::move(nested_relationship));
        }

        res = true;
    }

    return res;
}

void translation_unit_visitor::process_field(
    const cppast::cpp_member_variable &mv, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    bool template_instantiation_added_as_aggregation{false};

    auto type_name = cppast::to_string(mv.type());
    if (type_name.empty())
        type_name = "<<anonymous>>";

    class_member m{
        detail::cpp_access_specifier_to_access(as), mv.name(), type_name};

    if (mv.location().has_value()) {
        m.set_file(mv.location().value().file);
        m.set_line(mv.location().value().line);
    }

    if (mv.comment().has_value())
        m.add_decorators(decorators::parse(mv.comment().value()));

    if (m.skip())
        return;

    const auto &tr = cx::util::unreferenced(cppast::remove_cv(mv.type()));

    auto tr_declaration = cppast::to_string(tr);

    LOG_DBG("Processing field {} with unreferenced type of kind {}", mv.name(),
        cppast::to_string(tr.kind()));

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
            "Processing field with unexposed type {}", cppast::to_string(tr));
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
                relationship r{relationship_type, type, m.access(), m.name()};
                r.set_style(m.style_spec());

                auto [decorator_rtype, decorator_rmult] = m.get_relationship();
                if (decorator_rtype != relationship_t::kNone) {
                    r.set_type(decorator_rtype);
                    auto mult = util::split(decorator_rmult, ":", false);
                    if (mult.size() == 2) {
                        r.set_multiplicity_source(mult[0]);
                        r.set_multiplicity_destination(mult[1]);
                    }
                }

                LOG_DBG("Adding field relationship {} {} {} : {}",
                    r.destination(),
                    clanguml::common::model::to_string(r.type()), c.full_name(),
                    r.label());

                c.add_relationship(std::move(r));
            }
        }
    }

    c.add_member(std::move(m));
}

void translation_unit_visitor::process_anonymous_enum(
    const cppast::cpp_enum &en, class_ &c, cppast::cpp_access_specifier_kind as)
{
    for (const auto &ev : en) {
        if (ev.kind() == cppast::cpp_entity_kind::enum_value_t) {
            class_member m{
                detail::cpp_access_specifier_to_access(as), ev.name(), "enum"};
            c.add_member(std::move(m));
        }
    }
}

void translation_unit_visitor::process_static_field(
    const cppast::cpp_variable &mv, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_member m{detail::cpp_access_specifier_to_access(as), mv.name(),
        cppast::to_string(mv.type())};

    if (mv.location().has_value()) {
        m.set_file(mv.location().value().file);
        m.set_line(mv.location().value().line);
    }

    m.is_static(true);

    if (mv.comment().has_value())
        m.add_decorators(decorators::parse(mv.comment().value()));

    if (m.skip())
        return;

    c.add_member(std::move(m));
}

void translation_unit_visitor::process_method(
    const cppast::cpp_member_function &mf, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_method m{detail::cpp_access_specifier_to_access(as),
        util::trim(mf.name()), cppast::to_string(mf.return_type())};
    m.is_pure_virtual(cppast::is_pure(mf.virtual_info()));
    m.is_virtual(cppast::is_virtual(mf.virtual_info()));
    m.is_const(cppast::is_const(mf.cv_qualifier()));
    m.is_defaulted(false);
    m.is_static(false);

    if (mf.comment().has_value())
        m.set_comment(mf.comment().value());

    if (mf.location().has_value()) {
        m.set_file(mf.location().value().file);
        m.set_line(mf.location().value().line);
    }

    if (mf.comment().has_value())
        m.add_decorators(decorators::parse(mf.comment().value()));

    if (m.skip())
        return;

    const auto params = mf.parameters();
    for (auto &param : params)
        process_function_parameter(param, m, c);

    LOG_DBG("Adding method: {}", m.name());

    c.add_method(std::move(m));
}

void translation_unit_visitor::process_template_method(
    const cppast::cpp_function_template &mf, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    std::string type;
    if (mf.function().kind() == cppast::cpp_entity_kind::constructor_t)
        type = "void";
    else
        type = cppast::to_string(
            static_cast<const cppast::cpp_member_function &>(mf.function())
                .return_type());

    class_method m{detail::cpp_access_specifier_to_access(as),
        util::trim(mf.name()), type};
    m.is_pure_virtual(false);
    m.is_virtual(false);
    m.is_const(cppast::is_const(
        static_cast<const cppast::cpp_member_function &>(mf.function())
            .cv_qualifier()));
    m.is_defaulted(false);
    m.is_static(false);

    if (mf.comment().has_value())
        m.set_comment(mf.comment().value());

    if (mf.location().has_value()) {
        m.set_file(mf.location().value().file);
        m.set_line(mf.location().value().line);
    }

    if (mf.comment().has_value())
        m.add_decorators(decorators::parse(mf.comment().value()));

    if (m.skip())
        return;

    std::set<std::string> template_parameter_names;
    const auto template_params = mf.parameters();
    for (const auto &template_parameter : template_params) {
        template_parameter_names.emplace(template_parameter.name());
    }

    const auto params = mf.function().parameters();
    for (auto &param : params)
        process_function_parameter(param, m, c, template_parameter_names);

    LOG_DBG("Adding template method: {}", m.name());

    c.add_method(std::move(m));
}

void translation_unit_visitor::process_static_method(
    const cppast::cpp_function &mf, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_method m{detail::cpp_access_specifier_to_access(as),
        util::trim(mf.name()), cppast::to_string(mf.return_type())};
    m.is_pure_virtual(false);
    m.is_virtual(false);
    m.is_const(false);
    m.is_defaulted(false);
    m.is_static(true);

    if (mf.comment().has_value())
        m.set_comment(mf.comment().value());

    if (mf.location().has_value()) {
        m.set_file(mf.location().value().file);
        m.set_line(mf.location().value().line);
    }

    if (mf.comment().has_value())
        m.add_decorators(decorators::parse(mf.comment().value()));

    if (m.skip())
        return;

    for (auto &param : mf.parameters())
        process_function_parameter(param, m, c);

    LOG_DBG("Adding static method: {}", m.name());

    c.add_method(std::move(m));
}

void translation_unit_visitor::process_constructor(
    const cppast::cpp_constructor &mf, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_method m{detail::cpp_access_specifier_to_access(as),
        util::trim(mf.name()), "void"};
    m.is_pure_virtual(false);
    m.is_virtual(false);
    m.is_const(false);
    m.is_defaulted(false);
    m.is_static(false);

    if (mf.comment().has_value())
        m.set_comment(mf.comment().value());

    if (mf.location().has_value()) {
        m.set_file(mf.location().value().file);
        m.set_line(mf.location().value().line);
    }

    if (mf.comment().has_value())
        m.add_decorators(decorators::parse(mf.comment().value()));

    if (m.skip())
        return;

    for (auto &param : mf.parameters())
        process_function_parameter(param, m, c);

    c.add_method(std::move(m));
}

void translation_unit_visitor::process_destructor(
    const cppast::cpp_destructor &mf, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_method m{detail::cpp_access_specifier_to_access(as),
        util::trim(mf.name()), "void"};
    m.is_pure_virtual(false);
    m.is_virtual(cppast::is_virtual(mf.virtual_info()));
    m.is_const(false);
    m.is_defaulted(false);
    m.is_static(false);

    if (mf.comment().has_value())
        m.set_comment(mf.comment().value());

    if (mf.location().has_value()) {
        m.set_file(mf.location().value().file);
        m.set_line(mf.location().value().line);
    }

    c.add_method(std::move(m));
}

void translation_unit_visitor::process_function_parameter(
    const cppast::cpp_function_parameter &param, class_method &m, class_ &c,
    const std::set<std::string> &template_parameter_names)
{
    method_parameter mp;
    mp.set_name(param.name());

    if (param.comment().has_value())
        m.add_decorators(decorators::parse(param.comment().value()));

    if (mp.skip())
        return;

    const auto &param_type =
        cppast::remove_cv(cx::util::unreferenced(param.type()));
    if (param_type.kind() == cppast::cpp_type_kind::template_instantiation_t) {
        // TODO: Template instantiation parameters are not fully prefixed
        // so we have to deduce the correct namespace prefix of the
        // template which is being instantiated
        mp.set_type(cppast::to_string(param.type()));
    }
    else {
        mp.set_type(cppast::to_string(param.type()));
    }

    auto dv = param.default_value();
    if (dv) {
        switch (dv.value().kind()) {
        case cppast::cpp_expression_kind::literal_t:
            mp.set_default_value(
                static_cast<const cppast::cpp_literal_expression &>(dv.value())
                    .value());
            break;
        case cppast::cpp_expression_kind::unexposed_t:
            mp.set_default_value(
                static_cast<const cppast::cpp_unexposed_expression &>(
                    dv.value())
                    .expression()
                    .as_string());
            break;
        default:
            mp.set_default_value("{}");
        }
    }

    if (!mp.skip_relationship()) {
        // find relationship for the type
        std::vector<std::pair<std::string, relationship_t>> relationships;

        find_relationships(cppast::remove_cv(param.type()), relationships,
            relationship_t::kDependency);

        for (const auto &[type, relationship_type] : relationships) {
            auto [type_ns, type_name] = cx::util::split_ns(type);
            if (ctx.diagram().should_include(type_ns, type_name) &&
                (relationship_type != relationship_t::kNone) &&
                (type != c.name_and_ns())) {
                relationship r{relationship_t::kDependency, type};

                LOG_DBG("Adding field relationship {} {} {} : {}",
                    r.destination(),
                    clanguml::common::model::to_string(r.type()), c.full_name(),
                    r.label());

                c.add_relationship(std::move(r));
            }
        }

        // Also consider the container itself if it is a template instantiation
        // it's arguments could count as reference to relevant types
        const auto &t = cppast::remove_cv(cx::util::unreferenced(param.type()));
        if (t.kind() == cppast::cpp_type_kind::template_instantiation_t) {
            process_function_parameter_find_relationships_in_template(
                c, template_parameter_names, t);
        }
    }

    m.add_parameter(std::move(mp));
}

void translation_unit_visitor::
    process_function_parameter_find_relationships_in_template(class_ &c,
        const std::set<std::string> &template_parameter_names,
        const cppast::cpp_type &t)
{
    auto &template_instantiation_type =
        static_cast<const cppast::cpp_template_instantiation_type &>(t);

    const auto &full_name =
        cx::util::full_name(cppast::remove_cv(t), ctx.entity_index(), false);

    if (template_instantiation_type.primary_template()
            .get(ctx.entity_index())
            .size()) {
        // Check if the template arguments of this function param
        // are a subset of the method template params - if yes this is
        // not an instantiation but just a reference to an existing
        // template
        bool template_is_not_instantiation{false};
        if (template_instantiation_type.arguments_exposed()) {
            LOG_DBG("Processing template method argument exposed "
                    "parameters...");

            const auto targs = template_instantiation_type.arguments();
            for (const auto &template_argument : targs.value()) {
                if (!template_argument.type().has_value())
                    continue;

                const auto template_argument_name =
                    cppast::to_string(template_argument.type().value());
                if (template_parameter_names.count(template_argument_name) >
                    0) {
                    template_is_not_instantiation = true;
                    break;
                }
            }
        }
        else {
            LOG_DBG("Processing template method argument unexposed "
                    "parameters: ",
                template_instantiation_type.unexposed_arguments());
            // TODO: Process unexposed arguments by manually parsing the
            // arguments string
        }

        if (!ctx.diagram().should_include(ctx.get_namespace(),
                template_instantiation_type.primary_template()
                    .get(ctx.entity_index())[0]
                    .get()
                    .name())) {
            return;
        }

        if (template_is_not_instantiation) {
            relationship rr{relationship_t::kDependency, full_name};

            LOG_DBG("Template is not an instantiation - "
                    "only adding reference to template {}",
                full_name);

            LOG_DBG("Adding field template dependency relationship "
                    "{} {} {} "
                    ": {}",
                rr.destination(), common::model::to_string(rr.type()),
                c.full_name(), rr.label());

            c.add_relationship(std::move(rr));
        }
        else {
            // First check if tinst already exists
            auto tinst_ptr =
                build_template_instantiation(template_instantiation_type);
            const auto &tinst = *tinst_ptr;
            relationship rr{relationship_t::kDependency, tinst.full_name()};

            LOG_DBG("Adding field dependency relationship {} {} {} "
                    ": {}",
                rr.destination(), common::model::to_string(rr.type()),
                c.full_name(), rr.label());

            c.add_relationship(std::move(rr));

            if (ctx.diagram().should_include(tinst))
                ctx.diagram().add_class(std::move(tinst_ptr));
        }
    }
}

void translation_unit_visitor::process_template_type_parameter(
    const cppast::cpp_template_type_parameter &t, class_ &parent)
{
    template_parameter ct;
    ct.set_type("");
    ct.is_template_parameter(true);
    ct.set_name(t.name());
    ct.set_default_value("");
    ct.is_variadic(t.is_variadic());

    parent.add_template(std::move(ct));
}

void translation_unit_visitor::process_template_nontype_parameter(
    const cppast::cpp_non_type_template_parameter &t, class_ &parent)
{
    template_parameter ct;
    ct.set_type(cppast::to_string(t.type()));
    ct.is_template_parameter(false);
    ct.set_name(t.name());
    ct.set_default_value("");
    ct.is_variadic(t.is_variadic());

    parent.add_template(std::move(ct));
}

void translation_unit_visitor::process_template_template_parameter(
    const cppast::cpp_template_template_parameter &t, class_ &parent)
{
    template_parameter ct;
    ct.set_type("");
    ct.is_template_template_parameter(true);
    ct.set_name(t.name() + "<>");
    ct.set_default_value("");
    ct.is_variadic(t.is_variadic());

    parent.add_template(std::move(ct));
}

void translation_unit_visitor::process_friend(const cppast::cpp_friend &f,
    class_ &parent, cppast::cpp_access_specifier_kind as)
{
    // Only process friends to other classes or class templates
    if (!f.entity() ||
        ((f.entity().value().kind() != cppast::cpp_entity_kind::class_t) &&
            (f.entity().value().kind() !=
                cppast::cpp_entity_kind::class_template_t)))
        return;

    relationship r{relationship_t::kFriendship, "",
        detail::cpp_access_specifier_to_access(as), "<<friend>>"};

    if (f.comment().has_value())
        r.add_decorators(decorators::parse(f.comment().value()));

    if (r.skip() || r.skip_relationship())
        return;

    if (f.type()) {
        const auto [name_with_ns, name] =
            cx::util::split_ns(cppast::to_string(f.type().value()));
        if (!ctx.diagram().should_include(name_with_ns, name))
            return;

        LOG_DBG("Type friend declaration {}", name);

        // TODO: Destination should include namespace...
        r.set_destination(name);
    }
    else if (f.entity()) {
        std::string name = f.entity().value().name();

        if (f.entity().value().kind() ==
            cppast::cpp_entity_kind::class_template_t) {
            const auto &ft = static_cast<const cppast::cpp_class_template &>(
                f.entity().value());
            const auto &class_ = ft.class_();
            auto scope = cppast::cpp_scope_name(type_safe::ref(ft));
            if (class_.user_data() == nullptr) {
                spdlog::warn(
                    "Empty user data in friend class template: {}, {}, {}",
                    ft.name(),
                    fmt::ptr(reinterpret_cast<const void *>(&class_)),
                    scope.name());
                return;
            }

            LOG_DBG("Entity friend declaration {} ({})", name,
                static_cast<const char *>(ft.user_data()));

            name = static_cast<const char *>(ft.user_data());
        }
        else {
            LOG_DBG("Entity friend declaration {} ({},{})", name,
                cppast::is_templated(f.entity().value()),
                cppast::to_string(f.entity().value().kind()));

            const auto &maybe_definition =
                cppast::get_definition(ctx.entity_index(), f.entity().value());

            if (maybe_definition.has_value()) {
                const auto &friend_class = maybe_definition.value();

                auto entity_ns =
                    common::model::namespace_{cx::util::ns(friend_class)};

                name = cx::util::full_name(entity_ns, f.entity().value());
            }
        }

        if (!ctx.diagram().should_include(ctx.get_namespace(), name))
            return;

        r.set_destination(name);
    }
    else {
        LOG_DBG("Friend declaration points neither to type or entity.");
        return;
    }

    parent.add_relationship(std::move(r));
}

bool translation_unit_visitor::find_relationships(const cppast::cpp_type &t_,
    found_relationships_t &relationships,
    relationship_t relationship_hint) const
{
    bool found{false};

    const auto fn =
        cx::util::full_name(cppast::remove_cv(t_), ctx.entity_index(), false);

    LOG_DBG("Finding relationships for type {}, {}, {}", cppast::to_string(t_),
        cppast::to_string(t_.kind()), fn);

    relationship_t relationship_type = relationship_hint;
    const auto &t = cppast::remove_cv(cx::util::unreferenced(t_));

    auto name = cppast::to_string(t);

    if (t.kind() == cppast::cpp_type_kind::array_t) {
        found = find_relationships_in_array(relationships, t);
    }
    else if (t_.kind() == cppast::cpp_type_kind::pointer_t) {
        found =
            find_relationships_in_pointer(t_, relationships, relationship_hint);
    }
    else if (t_.kind() == cppast::cpp_type_kind::reference_t) {
        found = find_relationships_in_reference(
            t_, relationships, relationship_hint);
    }
    else if (cppast::remove_cv(t_).kind() ==
        cppast::cpp_type_kind::user_defined_t) {
        found = find_relationships_in_user_defined_type(
            t_, relationships, fn, relationship_type, t);
    }
    else if (t.kind() == cppast::cpp_type_kind::template_instantiation_t) {
        found = find_relationships_in_template_instantiation(
            t, fn, relationships, relationship_type);
    }

    return found;
}

bool translation_unit_visitor::find_relationships_in_template_instantiation(
    const cppast::cpp_type &t_, const std::string &fn,
    found_relationships_t &relationships,
    relationship_t relationship_type) const
{
    const auto &t = cppast::remove_cv(cx::util::unreferenced(t_));

    const auto &tinst =
        static_cast<const cppast::cpp_template_instantiation_type &>(t);

    auto name = cppast::to_string(t);

    bool found = false;

    if (!tinst.arguments_exposed()) {
        LOG_DBG("Template instantiation {} has no exposed arguments", name);

        return found;
    }

    assert(tinst.arguments().has_value());
    assert(tinst.arguments().value().size() > 0u);

    const auto args = tinst.arguments().value();

    const auto [ns, base_name] = cx::util::split_ns(fn);

    auto ns_and_name = ns | base_name;

    auto full_name = fmt::format("{}", fmt::join(ns_and_name, "::"));

    auto full_base_name = full_name.substr(0, full_name.find('<'));

    if (ctx.diagram().should_include(ns, name)) {
        LOG_DBG("User defined template instantiation: {} | {}",
            cppast::to_string(t_), cppast::to_string(t_.canonical()));

        if (relationship_type != relationship_t::kNone)
            relationships.emplace_back(cppast::to_string(t), relationship_type);
        else
            relationships.emplace_back(
                cppast::to_string(t), relationship_t::kAggregation);

        // Check if t_ has an alias in the alias index
        if (ctx.has_type_alias(fn)) {
            LOG_DBG("Find relationship in alias of {} | {}", fn,
                cppast::to_string(ctx.get_type_alias(fn).get()));
            found = find_relationships(
                ctx.get_type_alias(fn).get(), relationships, relationship_type);
        }
    }
    else {
        int argument_index = 0;
        auto relationship_hint = relationship_type;
        for (const auto &arg : args) {
            if (ctx.config().relationship_hints().count(full_base_name) > 0) {
                relationship_hint = ctx.config()
                                        .relationship_hints()
                                        .at(full_base_name)
                                        .get(argument_index);
            }

            if (arg.type().has_value()) {
                found = found ||
                    find_relationships(
                        arg.type().value(), relationships, relationship_hint);
            }

            argument_index++;
        }
    }

    return found;
}

bool translation_unit_visitor::find_relationships_in_user_defined_type(
    const cppast::cpp_type &t_, found_relationships_t &relationships,
    const std::string &fn, relationship_t &relationship_type,
    const cppast::cpp_type &t) const
{
    bool found{false};

    LOG_DBG("Finding relationships in user defined type: {} | {}",
        cppast::to_string(t_), cppast::to_string(t_.canonical()));

    if (relationship_type != relationship_t::kNone)
        relationships.emplace_back(cppast::to_string(t), relationship_type);
    else
        relationships.emplace_back(
            cppast::to_string(t), relationship_t::kAggregation);

    // Check if t_ has an alias in the alias index
    if (ctx.has_type_alias(fn)) {
        LOG_DBG("Find relationship in alias of {} | {}", fn,
            cppast::to_string(ctx.get_type_alias(fn).get()));
        if (cppast::to_string(t_) == fn)
            found = true;
        else
            found = find_relationships(
                ctx.get_type_alias(fn).get(), relationships, relationship_type);
    }
    return found;
}

bool translation_unit_visitor::find_relationships_in_reference(
    const cppast::cpp_type &t_, found_relationships_t &relationships,
    const relationship_t &relationship_hint) const
{
    bool found;
    auto &r = static_cast<const cppast::cpp_reference_type &>(t_);
    auto rt = relationship_t::kAssociation;
    if (r.reference_kind() == cppast::cpp_ref_rvalue) {
        rt = relationship_t::kAggregation;
    }
    if (relationship_hint == relationship_t::kDependency)
        rt = relationship_hint;
    found = find_relationships(r.referee(), relationships, rt);
    return found;
}

bool translation_unit_visitor::find_relationships_in_pointer(
    const cppast::cpp_type &t_, found_relationships_t &relationships,
    const relationship_t &relationship_hint) const
{
    bool found;
    auto &p = static_cast<const cppast::cpp_pointer_type &>(t_);
    auto rt = relationship_t::kAssociation;
    if (relationship_hint == relationship_t::kDependency)
        rt = relationship_hint;
    found = find_relationships(p.pointee(), relationships, rt);
    return found;
}

bool translation_unit_visitor::find_relationships_in_array(
    found_relationships_t &relationships, const cppast::cpp_type &t) const
{
    bool found;
    auto &a = static_cast<const cppast::cpp_array_type &>(t);
    found = find_relationships(
        a.value_type(), relationships, relationship_t::kAggregation);
    return found;
}

bool translation_unit_visitor::find_relationships_in_unexposed_template_params(
    const template_parameter &ct, found_relationships_t &relationships) const
{
    bool found{false};
    LOG_DBG("Finding relationships in user defined type: {}",
        ct.to_string(ctx.config().using_namespace(), false));

    auto type_with_namespace = ctx.get_name_with_namespace(ct.type());

    if (!type_with_namespace.has_value()) {
        // Couldn't find declaration of this type
        type_with_namespace = common::model::namespace_{ct.type()};
    }

    if (ctx.diagram().should_include(type_with_namespace.value().to_string())) {
        relationships.emplace_back(type_with_namespace.value().to_string(),
            relationship_t::kDependency);
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
    const cppast::cpp_template_instantiation_type &t,
    std::optional<clanguml::class_diagram::model::class_ *> parent)
{
    //
    // Create class_ instance to hold the template instantiation
    //
    auto tinst_ptr = std::make_unique<class_>(ctx.config().using_namespace());
    auto &tinst = *tinst_ptr;
    std::string full_template_name;

    auto tr_declaration = cppast::to_string(t);

    //
    // If this is an alias - resolve the alias target
    //
    const auto &unaliased =
        static_cast<const cppast::cpp_template_instantiation_type &>(
            resolve_alias(t));
    auto t_unaliased_declaration = cppast::to_string(unaliased);

    bool t_is_alias = t_unaliased_declaration != tr_declaration;

    //
    // Here we'll hold the template base params to replace with the
    // instantiated values
    //
    std::deque<std::tuple<std::string, int, bool>> template_base_params{};

    tinst.set_namespace(ctx.get_namespace());

    std::string tinst_full_name;

    //
    // Typically, every template instantiation should have a
    // primary_template(), which should also be generated here if it doesn't
    // exist yet in the model
    //
    if (t_is_alias &&
        unaliased.primary_template().get(ctx.entity_index()).size()) {
        tinst_full_name = cppast::to_string(unaliased);
        build_template_instantiation_primary_template(
            unaliased, tinst, template_base_params, parent, full_template_name);
    }
    else if (t.primary_template().get(ctx.entity_index()).size()) {
        tinst_full_name = cppast::to_string(t);
        build_template_instantiation_primary_template(
            t, tinst, template_base_params, parent, full_template_name);
    }
    else {
        LOG_DBG("Template instantiation {} has no primary template?",
            cppast::to_string(t));

        tinst_full_name = cppast::to_string(t);
        full_template_name = cppast::to_string(t);
    }

    LOG_DBG("Building template instantiation for {}", full_template_name);

    //
    // Extract namespace from base template name
    //
    const auto [ns, name] = cx::util::split_ns(tinst_full_name);
    tinst.set_name(name);
    if (ns.is_empty())
        tinst.set_namespace(ctx.get_namespace());
    else
        tinst.set_namespace(ns);

    tinst.is_template_instantiation(true);
    tinst.is_alias(t_is_alias);

    //
    // Process exposed template arguments - if any
    //
    if (t.arguments_exposed()) {
        auto arg_index = 0U;
        // We can figure this only when we encounter variadic param in
        // the list of template params, from then this variable is true
        // and we can process following template parameters as belonging
        // to the variadic tuple
        auto has_variadic_params = false;

        const auto targs = t.arguments().value();
        for (const auto &targ : targs) {
            template_parameter ct;
            if (targ.type()) {
                build_template_instantiation_process_type_argument(
                    parent, tinst, targ.type().value(), ct);
            }
            else if (targ.expression()) {
                build_template_instantiation_process_expression_argument(
                    targ, ct);
            }

            // In case any of the template arguments are base classes, add
            // them as parents of the current template instantiation class
            if (template_base_params.size() > 0) {
                has_variadic_params =
                    build_template_instantiation_add_base_classes(tinst,
                        template_base_params, arg_index, has_variadic_params,
                        ct);
            }

            LOG_DBG("Adding template argument '{}'", ct.to_string({}, false));

            tinst.add_template(std::move(ct));
        }
    }

    // Add instantiation relationship to primary template of this
    // instantiation
    const auto &tt = cppast::remove_cv(cx::util::unreferenced(t));
    auto fn = cx::util::full_name(tt, ctx.entity_index(), false);
    fn = util::split(fn, "<")[0];

    std::string destination;
    if (ctx.has_type_alias(fn)) {
        // If this is a template alias - set the instantiation
        // relationship to the first alias target
        destination = cppast::to_string(ctx.get_type_alias(fn).get());
    }
    else {
        std::string best_match_full_name{};

        int best_match = 0;
        // First try to find the best match for this template in partially
        // specialized templates
        for (const auto c : ctx.diagram().classes()) {
            if (c == tinst)
                continue;

            auto match = c->calculate_template_specialization_match(
                tinst, full_template_name);

            if (match > best_match) {
                best_match = match;
                best_match_full_name = c->full_name(false);
            }
        }

        if (!best_match_full_name.empty())
            destination = best_match_full_name;
        else
            // Otherwise point to the base template
            destination = tinst.base_template();
    }

    relationship r{relationship_t::kInstantiation, destination};
    tinst.add_relationship(std::move(r));

    return tinst_ptr;
}

bool translation_unit_visitor::build_template_instantiation_add_base_classes(
    class_ &tinst,
    std::deque<std::tuple<std::string, int, bool>> &template_base_params,
    int arg_index, bool variadic_params, const template_parameter &ct) const
{
    bool add_template_argument_as_base_class = false;

    auto [arg_name, is_variadic, index] = template_base_params.front();
    if (variadic_params)
        add_template_argument_as_base_class = true;
    else {
        variadic_params = is_variadic;
        if (arg_index == index) {
            add_template_argument_as_base_class = true;
            template_base_params.pop_front();
        }
    }

    if (add_template_argument_as_base_class) {
        LOG_DBG("Adding template argument as base class '{}'",
            ct.to_string({}, false));

        class_parent cp;
        cp.set_access(access_t::kPublic);
        cp.set_name(ct.to_string({}, false));

        tinst.add_parent(std::move(cp));
    }

    return variadic_params;
}

void translation_unit_visitor::
    build_template_instantiation_process_expression_argument(
        const cppast::cpp_template_argument &targ, template_parameter &ct) const
{
    const auto exp = targ.expression();
    if (exp.value().kind() == cppast::cpp_expression_kind::literal_t)
        ct.set_type(
            static_cast<const cppast::cpp_literal_expression &>(exp.value())
                .value());
    else if (exp.value().kind() == cppast::cpp_expression_kind::unexposed_t)
        ct.set_type(
            static_cast<const cppast::cpp_unexposed_expression &>(exp.value())
                .expression()
                .as_string());

    LOG_DBG("Template argument is an expression {}", ct.name());
}

void translation_unit_visitor::
    build_template_instantiation_process_type_argument(
        const std::optional<clanguml::class_diagram::model::class_ *> &parent,
        class_ &tinst, const cppast::cpp_type &targ_type,
        template_parameter &ct)
{
    auto full_name = cx::util::full_name(
        cppast::remove_cv(cx::util::unreferenced(targ_type)),
        ctx.entity_index(), false);

    auto [fn_ns, fn_name] = cx::util::split_ns(full_name);
    auto template_argument_kind = targ_type.kind();

    if (template_argument_kind == cppast::cpp_type_kind::unexposed_t) {
        // Here we're on our own - just make a best guess
        if (!full_name.empty() && !util::contains(full_name, "<") &&
            !util::contains(full_name, ":") && std::isupper(full_name.at(0)))
            ct.is_template_parameter(true);
        else
            ct.is_template_parameter(false);

        ct.set_name(full_name);
    }
    else if (template_argument_kind ==
        cppast::cpp_type_kind::template_parameter_t) {
        ct.is_template_parameter(true);
        ct.set_name(full_name);
    }
    else if (template_argument_kind == cppast::cpp_type_kind::builtin_t) {
        ct.is_template_parameter(false);
        ct.set_type(full_name);
    }
    else if (template_argument_kind ==
        cppast::cpp_type_kind::template_instantiation_t) {

        // Check if this template should be simplified (e.g. system
        // template aliases such as std:basic_string<char> should be simply
        // std::string)
        if (simplify_system_template(ct, full_name)) {
            return;
        }

        const auto &nested_template_parameter =
            static_cast<const cppast::cpp_template_instantiation_type &>(
                targ_type);

        auto [tinst_ns, tinst_name] =
            cx::util::split_ns(tinst.full_name(false));

        ct.set_name(full_name.substr(0, full_name.find('<')));

        assert(!ct.name().empty());

        auto nested_tinst =
            build_template_instantiation(nested_template_parameter,
                ctx.diagram().should_include(tinst_ns, tinst_name)
                    ? std::make_optional(&tinst)
                    : parent);

        assert(nested_tinst);

        for (const auto &t : nested_tinst->templates())
            ct.add_template_param(t);

        relationship tinst_dependency{
            relationship_t::kDependency, nested_tinst->full_name()};

        auto nested_tinst_full_name = nested_tinst->full_name(false);

        auto [nested_tinst_ns, nested_tinst_name] =
            cx::util::split_ns(nested_tinst_full_name);

        if (ctx.diagram().should_include(nested_tinst_ns, nested_tinst_name)) {

            ctx.diagram().add_class(std::move(nested_tinst));
        }

        if (ctx.diagram().should_include(tinst_ns, tinst_name)
            // TODO: check why this breaks t00033:
            //    && ctx.config().should_include(
            //       cx::util::split_ns(tinst_dependency.destination()))
        ) {
            LOG_DBG("Creating nested template dependency to template "
                    "instantiation {}, {} -> {}",
                full_name, tinst.full_name(), tinst_dependency.destination());

            tinst.add_relationship(std::move(tinst_dependency));
        }
        else if (parent) {
            LOG_DBG("Creating nested template dependency to parent "
                    "template "
                    "instantiation {}, {} -> {}",
                full_name, (*parent)->full_name(),
                tinst_dependency.destination());

            (*parent)->add_relationship(std::move(tinst_dependency));
        }
        else {
            LOG_DBG("No nested template dependency to template "
                    "instantiation: {}, {} -> {}",
                full_name, tinst.full_name(), tinst_dependency.destination());
        }
    }
    else if (template_argument_kind == cppast::cpp_type_kind::function_t) {
        const auto &function_argument =
            static_cast<const cppast::cpp_function_type &>(targ_type);

        // Search for relationships in argument return type
        // TODO...

        // Build instantiations of each of the arguments
        for (const auto &arg : function_argument.parameter_types()) {
            template_parameter ctt;

            build_template_instantiation_process_type_argument(
                parent, tinst, arg, ctt);
        }
    }
    else if (template_argument_kind == cppast::cpp_type_kind::user_defined_t) {
        relationship tinst_dependency{relationship_t::kDependency,
            cx::util::full_name(
                cppast::remove_cv(cx::util::unreferenced(targ_type)),
                ctx.entity_index(), false)};

        LOG_DBG("Creating nested template dependency to user defined "
                "type {} -> {}",
            tinst.full_name(), tinst_dependency.destination());

        ct.set_name(full_name);

        if (ctx.diagram().should_include(fn_ns, fn_name)) {
            tinst.add_relationship(std::move(tinst_dependency));
        }
        else if (parent) {
            (*parent)->add_relationship(std::move(tinst_dependency));
        }
    }
}

void translation_unit_visitor::build_template_instantiation_primary_template(
    const cppast::cpp_template_instantiation_type &t, class_ &tinst,
    std::deque<std::tuple<std::string, int, bool>> &template_base_params,
    std::optional<clanguml::class_diagram::model::class_ *> &parent,
    std::string &full_template_name) const
{
    const auto &primary_template_ref =
        static_cast<const cppast::cpp_class_template &>(
            t.primary_template().get(ctx.entity_index())[0].get())
            .class_();

    if (parent)
        LOG_DBG("Template parent is {}", (*parent)->full_name());
    else
        LOG_DBG("Template parent is empty");

    full_template_name =
        cx::util::full_name(ctx.get_namespace(), primary_template_ref);

    LOG_DBG("Found template instantiation: "
            "type={}, canonical={}, primary_template={}, full_"
            "template={}",
        cppast::to_string(t), cppast::to_string(t.canonical()),
        t.primary_template().name(), full_template_name);

    if (full_template_name.back() == ':') {
        tinst.set_name(full_template_name + tinst.name());
    }

    std::vector<std::pair<std::string, bool>> template_parameter_names{};
    if (primary_template_ref.scope_name().has_value()) {
        for (const auto &tp :
            primary_template_ref.scope_name().value().template_parameters()) {
            template_parameter_names.emplace_back(tp.name(), tp.is_variadic());
        }
    }

    // Check if the primary template has any base classes
    int base_index = 0;
    for (const auto &base : primary_template_ref.bases()) {
        if (base.kind() == cppast::cpp_entity_kind::base_class_t) {
            const auto &base_class =
                static_cast<const cppast::cpp_base_class &>(base);

            const auto base_class_name = cppast::to_string(base_class.type());

            LOG_DBG("Found template instantiation base: {}, {}, {}",
                cppast::to_string(base.kind()), base_class_name, base_index);

            // Check if any of the primary template arguments has a
            // parameter equal to this type
            auto it = std::find_if(template_parameter_names.begin(),
                template_parameter_names.end(),
                [&base_class_name](
                    const auto &p) { return p.first == base_class_name; });

            if (it != template_parameter_names.end()) {
                // Found base class which is a template parameter
                LOG_DBG("Found base class which is a template parameter "
                        "{}, {}, {}",
                    it->first, it->second,
                    std::distance(template_parameter_names.begin(), it));

                template_base_params.emplace_back(it->first, it->second,
                    std::distance(template_parameter_names.begin(), it));
            }
            else {
                // This is a regular base class - it is handled by
                // process_template
            }
        }
        base_index++;
    }

    if (primary_template_ref.user_data()) {
        tinst.set_base_template(
            static_cast<const char *>(primary_template_ref.user_data()));
        LOG_DBG("Primary template ref set to: {}", tinst.base_template());
    }
    else
        LOG_DBG(
            "No user data for base template {}", primary_template_ref.name());
}

const cppast::cpp_type &translation_unit_visitor::resolve_alias(
    const cppast::cpp_type &type)
{
    const auto &raw_type = cppast::remove_cv(cx::util::unreferenced(type));
    auto type_full_name =
        cx::util::full_name(raw_type, ctx.entity_index(), false);

    if (util::contains(type_full_name, "<"))
        type_full_name = util::split(type_full_name, "<")[0];

    auto type_full_name_in_current_ns = ctx.get_namespace();
    type_full_name_in_current_ns |= common::model::namespace_{type_full_name};

    if (ctx.has_type_alias_template(type_full_name)) {
        return ctx.get_type_alias(type_full_name).get();
    }
    else if (ctx.has_type_alias_template(
                 type_full_name_in_current_ns.to_string())) {
        return ctx.get_type_alias(type_full_name_in_current_ns.to_string())
            .get();
    }
    else if (ctx.has_type_alias(type_full_name)) {
        return ctx.get_type_alias_final(raw_type).get();
    }

    return type;
}

const cppast::cpp_type &translation_unit_visitor::resolve_alias_template(
    const cppast::cpp_type &type)
{
    const auto &raw_type = cppast::remove_cv(cx::util::unreferenced(type));
    const auto type_full_name =
        cx::util::full_name(raw_type, ctx.entity_index(), false);
    if (ctx.has_type_alias_template(type_full_name)) {
        return ctx.get_type_alias_template(type_full_name).get();
    }

    return type;
}

bool translation_unit_visitor::simplify_system_template(
    template_parameter &ct, const std::string &full_name)
{
    if (ctx.config().template_aliases().count(full_name) > 0) {
        ct.set_name(ctx.config().template_aliases().at(full_name));
        return true;
    }
    else
        return false;
}
}
