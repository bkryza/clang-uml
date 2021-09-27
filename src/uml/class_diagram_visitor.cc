/**
 * src/uml/class_diagram_visitor.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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
#include "class_diagram_visitor.h"

#include <cppast/cpp_alias_template.hpp>
#include <cppast/cpp_array_type.hpp>
#include <cppast/cpp_class_template.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_friend.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <cppast/cpp_namespace.hpp>
#include <cppast/cpp_template.hpp>
#include <cppast/cpp_type_alias.hpp>
#include <cppast/cpp_variable.hpp>
#include <spdlog/spdlog.h>

#include <deque>

namespace clanguml {
namespace visitor {
namespace class_diagram {

using clanguml::model::class_diagram::class_;
using clanguml::model::class_diagram::class_member;
using clanguml::model::class_diagram::class_method;
using clanguml::model::class_diagram::class_parent;
using clanguml::model::class_diagram::class_relationship;
using clanguml::model::class_diagram::class_template;
using clanguml::model::class_diagram::diagram;
using clanguml::model::class_diagram::enum_;
using clanguml::model::class_diagram::method_parameter;
using clanguml::model::class_diagram::relationship_t;
using clanguml::model::class_diagram::scope_t;
using clanguml::model::class_diagram::type_alias;

namespace detail {
scope_t cpp_access_specifier_to_scope(cppast::cpp_access_specifier_kind as)
{
    scope_t res = scope_t::kPublic;
    switch (as) {
    case cppast::cpp_access_specifier_kind::cpp_public:
        res = scope_t::kPublic;
        break;
    case cppast::cpp_access_specifier_kind::cpp_private:
        res = scope_t::kPrivate;
        break;
    case cppast::cpp_access_specifier_kind::cpp_protected:
        res = scope_t::kProtected;
        break;
    default:
        break;
    }

    return res;
}
}

//
// tu_context
//
tu_context::tu_context(cppast::cpp_entity_index &idx,
    clanguml::model::class_diagram::diagram &d_,
    const clanguml::config::class_diagram &config_)
    : entity_index{idx}
    , d{d_}
    , config{config_}
{
}

bool tu_context::has_type_alias(const std::string &full_name) const
{
    bool res = alias_index.find(full_name) != alias_index.end();
    LOG_DBG("Alias {} {} found in index", full_name, res ? "" : "not");
    return res;
}

void tu_context::add_type_alias(const std::string &full_name,
    type_safe::object_ref<const cppast::cpp_type> &&ref)
{
    if (!has_type_alias(full_name)) {
        LOG_DBG("Stored type alias: {} -> {} ", full_name,
            cppast::to_string(ref.get()));
        alias_index.emplace(full_name, std::move(ref));
    }
}

type_safe::object_ref<const cppast::cpp_type> tu_context::get_type_alias(
    const std::string &full_name) const
{
    assert(has_type_alias(full_name));

    return alias_index.at(full_name);
}

type_safe::object_ref<const cppast::cpp_type> tu_context::get_type_alias_final(
    const cppast::cpp_type &t) const
{
    const auto fn =
        cx::util::full_name(cppast::remove_cv(t), entity_index, false);

    if (has_type_alias(fn)) {
        return get_type_alias_final(alias_index.at(fn).get());
    }

    return type_safe::ref(t);
}

bool tu_context::has_type_alias_template(const std::string &full_name) const
{
    bool res =
        alias_template_index.find(full_name) != alias_template_index.end();
    LOG_DBG("Alias template {} {} found in index", full_name, res ? "" : "not");
    return res;
}

void tu_context::add_type_alias_template(const std::string &full_name,
    type_safe::object_ref<const cppast::cpp_type> &&ref)
{
    if (!has_type_alias_template(full_name)) {
        LOG_DBG("Stored type alias template for: {} ", full_name);
        alias_template_index.emplace(full_name, std::move(ref));
    }
}

type_safe::object_ref<const cppast::cpp_type>
tu_context::get_type_alias_template(const std::string &full_name) const
{
    assert(has_type_alias_template(full_name));

    return alias_template_index.at(full_name);
}

//
// element_visitor_context
//
template <typename T>
element_visitor_context<T>::element_visitor_context(
    clanguml::model::class_diagram::diagram &d_, T &e)
    : element(e)
    , d{d_}
{
}

//
// tu_visitor
//

tu_visitor::tu_visitor(cppast::cpp_entity_index &idx_,
    clanguml::model::class_diagram::diagram &d_,
    const clanguml::config::class_diagram &config_)
    : ctx{idx_, d_, config_}
{
}

void tu_visitor::operator()(const cppast::cpp_entity &file)
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
                        !ns_declaration.is_inline())
                        ctx.namespace_.push_back(e.name());
                }
                else {
                    LOG_DBG("========== Leaving '{}' - {}", e.name(),
                        cppast::to_string(e.kind()));

                    const auto &ns_declaration =
                        static_cast<const cppast::cpp_namespace &>(e);
                    if (!ns_declaration.is_anonymous() &&
                        !ns_declaration.is_inline())
                        ctx.namespace_.pop_back();
                }
            }
            else if (e.kind() ==
                cppast::cpp_entity_kind::class_template_specialization_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.namespace_, e),
                    cppast::to_string(e.kind()));

                auto &tspec = static_cast<
                    const cppast::cpp_class_template_specialization &>(e);

                process_class_declaration(
                    tspec.class_(), type_safe::ref(tspec));
            }
            else if (e.kind() == cppast::cpp_entity_kind::class_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.namespace_, e),
                    cppast::to_string(e.kind()));

                auto &cls = static_cast<const cppast::cpp_class &>(e);
                if (cppast::get_definition(ctx.entity_index, cls)) {
                    auto &clsdef = static_cast<const cppast::cpp_class &>(
                        cppast::get_definition(ctx.entity_index, cls).value());
                    if (&cls != &clsdef) {
                        LOG_DBG("Forward declaration of class {} - skipping...",
                            cls.name());
                        return;
                    }
                }

                if (ctx.config.should_include(
                        cx::util::fully_prefixed(ctx.namespace_, cls)))
                    process_class_declaration(cls);
            }
            else if (e.kind() == cppast::cpp_entity_kind::enum_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.namespace_, e),
                    cppast::to_string(e.kind()));

                auto &enm = static_cast<const cppast::cpp_enum &>(e);

                if (ctx.config.should_include(
                        cx::util::fully_prefixed(ctx.namespace_, enm)))
                    process_enum_declaration(enm);
            }
            else if (e.kind() == cppast::cpp_entity_kind::type_alias_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.namespace_, e),
                    cppast::to_string(e.kind()));

                auto &ta = static_cast<const cppast::cpp_type_alias &>(e);
                type_alias t;
                t.alias = cx::util::full_name(ctx.namespace_, ta);
                t.underlying_type = cx::util::full_name(ta.underlying_type(),
                    ctx.entity_index, cx::util::is_inside_class(e));

                ctx.add_type_alias(cx::util::full_name(ctx.namespace_, ta),
                    type_safe::ref(ta.underlying_type()));

                ctx.d.add_type_alias(std::move(t));
            }
            else if (e.kind() == cppast::cpp_entity_kind::alias_template_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.namespace_, e),
                    cppast::to_string(e.kind()));

                auto &at = static_cast<const cppast::cpp_alias_template &>(e);

                class_ tinst = build_template_instantiation(static_cast<
                    const cppast::cpp_template_instantiation_type &>(
                    at.type_alias().underlying_type()));

                ctx.d.add_class(std::move(tinst));
            }
        });
}

void tu_visitor::process_enum_declaration(const cppast::cpp_enum &enm)
{
    if (enm.name().empty()) {
        // Anonymous enum values should be rendered as class fields
        // with type enum
        return;
    }

    enum_ e{ctx.config.using_namespace};
    e.set_name(cx::util::full_name(ctx.namespace_, enm));

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
            class_relationship containment;
            containment.type = relationship_t::kContainment;
            containment.destination =
                cx::util::full_name(ctx.namespace_, cur.value());
            containment.scope = scope_t::kNone;
            e.add_relationship(std::move(containment));

            LOG_DBG("Added relationship {} +-- {}", e.name(),
                containment.destination);
            break;
        }
    }

    ctx.d.add_enum(std::move(e));
}

void tu_visitor::process_class_declaration(const cppast::cpp_class &cls,
    type_safe::optional_ref<const cppast::cpp_template_specialization> tspec)
{
    class_ c{ctx.config.using_namespace};
    c.is_struct(cls.class_kind() == cppast::cpp_class_kind::struct_t);
    c.set_name(cx::util::full_name(ctx.namespace_, cls));

    if (cls.comment().has_value())
        c.add_decorators(decorators::parse(cls.comment().value()));

    cppast::cpp_access_specifier_kind last_access_specifier =
        cppast::cpp_access_specifier_kind::cpp_private;

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

            process_friend(fr, c);
        }
        else if (cppast::is_friended(child)) {
            auto &fr =
                static_cast<const cppast::cpp_friend &>(child.parent().value());

            LOG_DBG("Found friend template: {}", child.name());

            process_friend(fr, c);
        }
        else {
            LOG_DBG("Found some other class child: {} ({})", child.name(),
                cppast::to_string(child.kind()));
        }
    }

    // Process class bases
    for (auto &base : cls.bases()) {
        class_parent cp;
        cp.name = clanguml::cx::util::fully_prefixed(ctx.namespace_, base);
        cp.is_virtual = base.is_virtual();

        switch (base.access_specifier()) {
        case cppast::cpp_access_specifier_kind::cpp_private:
            cp.access = class_parent::access_t::kPrivate;
            break;
        case cppast::cpp_access_specifier_kind::cpp_public:
            cp.access = class_parent::access_t::kPublic;
            break;
        case cppast::cpp_access_specifier_kind::cpp_protected:
            cp.access = class_parent::access_t::kProtected;
            break;
        default:
            cp.access = class_parent::access_t::kPublic;
        }

        LOG_DBG("Found base class {} for class {}", cp.name, c.name());

        c.add_parent(std::move(cp));
    }

    // Process class template arguments
    if (cppast::is_templated(cls)) {
        LOG_DBG("Processing class template parameters...");
        auto scope = cppast::cpp_scope_name(type_safe::ref(cls));
        // Even if this is a template the scope.is_templated() returns
        // false when the template parameter list is empty
        if (scope.is_templated()) {
            for (const auto &tp : scope.template_parameters()) {
                if (tp.kind() ==
                    cppast::cpp_entity_kind::template_type_parameter_t) {
                    LOG_DBG("Processing template type parameter {}", tp.name());
                    process_template_type_parameter(
                        static_cast<
                            const cppast::cpp_template_type_parameter &>(tp),
                        c);
                }
                else if (tp.kind() ==
                    cppast::cpp_entity_kind::non_type_template_parameter_t) {
                    LOG_DBG(
                        "Processing template nontype parameter {}", tp.name());
                    process_template_nontype_parameter(
                        static_cast<
                            const cppast::cpp_non_type_template_parameter &>(
                            tp),
                        c);
                }
                else if (tp.kind() ==
                    cppast::cpp_entity_kind::template_template_parameter_t) {
                    LOG_DBG(
                        "Processing template template parameter {}", tp.name());
                    process_template_template_parameter(
                        static_cast<
                            const cppast::cpp_template_template_parameter &>(
                            tp),
                        c);
                }
            }
        }
        else {
            LOG_WARN("Class {} is templated but it's scope {} is not - "
                     "probably this is a specialization",
                cls.name(), scope.name());

            // Add specialization arguments
            if (tspec) {
                if (!tspec.value().arguments_exposed()) {
                    // Create template specialization with unexposed arguments
                    auto ua = tspec.value().unexposed_arguments().as_string();

                    // Naive parse of template arguments:
                    auto toks = util::split(ua, ",");
                    for (const auto &t : toks) {
                        class_template ct;
                        ct.type = t;
                        ct.default_value = "";
                        ct.is_variadic = false;
                        ct.name = "";
                        c.add_template(std::move(ct));

                        const auto &primary_template_ref =
                            static_cast<const cppast::cpp_class_template &>(
                                tspec.value()
                                    .primary_template()
                                    .get(ctx.entity_index)[0]
                                    .get())
                                .class_();

                        if (primary_template_ref.user_data()) {
                            auto base_template_full_name =
                                static_cast<const char *>(
                                    primary_template_ref.user_data());
                            LOG_DBG("Primary template ref set to: {}",
                                base_template_full_name);
                            // Add template specialization/instantiation
                            // relationship
                            class_relationship r;
                            r.type = relationship_t::kInstantiation;
                            r.label = "";
                            r.destination = base_template_full_name;
                            r.scope = scope_t::kNone;
                            c.add_relationship(std::move(r));
                        }
                        else {
                            LOG_WARN("No user data for base template {}",
                                primary_template_ref.name());
                        }
                    }
                }
                else {
                    for (auto &tp : tspec.value().parameters()) {
                        switch (tp.kind()) {
                        case cppast::cpp_entity_kind::
                            template_type_parameter_t: {
                            LOG_DBG("Processing template type parameter {}",
                                tp.name());
                            process_template_type_parameter(
                                static_cast<const cppast::
                                        cpp_template_type_parameter &>(tp),
                                c);
                        } break;
                        case cppast::cpp_entity_kind::
                            non_type_template_parameter_t: {
                            LOG_DBG("Processing template nontype parameter {}",
                                tp.name());
                            process_template_nontype_parameter(
                                static_cast<const cppast::
                                        cpp_non_type_template_parameter &>(tp),
                                c);
                        } break;
                        case cppast::cpp_entity_kind::
                            template_template_parameter_t: {
                            LOG_DBG("Processing template template parameter {}",
                                tp.name());
                            process_template_template_parameter(
                                static_cast<const cppast::
                                        cpp_template_template_parameter &>(tp),
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
            }
            else {
                LOG_DBG("Skipping template class declaration which has only "
                        "unexposed arguments but no tspec provided");
                return;
            }
        }
    }

    // Find if class is contained in another class
    for (auto cur = cls.parent(); cur; cur = cur.value().parent()) {
        // find nearest parent class, if any
        if (cur.value().kind() == cppast::cpp_entity_kind::class_t) {
            class_relationship containment;
            containment.type = relationship_t::kContainment;
            containment.destination =
                cx::util::full_name(ctx.namespace_, cur.value());
            c.add_relationship(std::move(containment));

            LOG_DBG("Added relationship {} +-- {}", c.full_name(),
                containment.destination);

            break;
        }
    }

    cls.set_user_data(strdup(c.full_name().c_str()));

    LOG_DBG("Setting user data for class {}, {}",
        static_cast<const char *>(cls.user_data()),
        fmt::ptr(reinterpret_cast<const void *>(&cls)));

    ctx.d.add_class(std::move(c));
}

bool tu_visitor::process_field_with_template_instantiation(
    const cppast::cpp_member_variable &mv, const cppast::cpp_type &tr,
    class_ &c, class_member &m, cppast::cpp_access_specifier_kind as)
{
    LOG_DBG("Processing field with template instantiation type {}",
        cppast::to_string(tr));

    bool res = false;

    const auto &template_instantiation_type =
        static_cast<const cppast::cpp_template_instantiation_type &>(tr);

    const auto &unaliased =
        static_cast<const cppast::cpp_template_instantiation_type &>(
            resolve_alias(template_instantiation_type));

    class_ tinst = build_template_instantiation(unaliased);

    // Infer the relationship of this field to the template
    // instantiation
    class_relationship rr;
    rr.destination = tinst.full_name();
    if (mv.type().kind() == cppast::cpp_type_kind::pointer_t ||
        mv.type().kind() == cppast::cpp_type_kind::reference_t)
        rr.type = relationship_t::kAssociation;
    else
        rr.type = relationship_t::kAggregation;
    rr.label = mv.name();
    rr.scope = detail::cpp_access_specifier_to_scope(as);
    rr.set_style(m.style_spec());

    // Process field decorators
    auto [decorator_rtype, decorator_rmult] = m.relationship();
    if (decorator_rtype != relationship_t::kNone) {
        rr.type = decorator_rtype;
        auto mult = util::split(decorator_rmult, ":");
        if (mult.size() == 2) {
            rr.multiplicity_source = mult[0];
            rr.multiplicity_destination = mult[1];
        }
    }

    if (ctx.config.should_include(tinst.name())) {
        LOG_DBG("Adding field instantiation relationship {} {} {} : {}",
            rr.destination, model::class_diagram::to_string(rr.type),
            c.full_name(), rr.label);

        c.add_relationship(std::move(rr));

        res = true;

        LOG_DBG("Created template instantiation: {}", tinst.full_name());

        ctx.d.add_class(std::move(tinst));
    }

    return res;
}

void tu_visitor::process_field(const cppast::cpp_member_variable &mv, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    bool template_instantiation_added_as_aggregation{false};

    class_member m{detail::cpp_access_specifier_to_scope(as), mv.name(),
        cppast::to_string(mv.type())};

    if (mv.comment().has_value())
        m.add_decorators(decorators::parse(mv.comment().value()));

    if (m.skip())
        return;

    auto &tr = cx::util::unreferenced(cppast::remove_cv(mv.type()));

    LOG_DBG("Processing field {} with unreferenced type of kind {}", mv.name(),
        tr.kind());

    if (tr.kind() == cppast::cpp_type_kind::builtin_t) {
        LOG_DBG("Builtin type found for field: {}", m.name());
    }
    else if (tr.kind() == cppast::cpp_type_kind::user_defined_t) {
        LOG_DBG("Processing user defined type field {} {}",
            cppast::to_string(tr), mv.name());
    }
    else if (tr.kind() == cppast::cpp_type_kind::template_instantiation_t) {
        template_instantiation_added_as_aggregation =
            process_field_with_template_instantiation(
                mv, resolve_alias(tr), c, m, as);
    }
    else if (tr.kind() == cppast::cpp_type_kind::unexposed_t) {
        LOG_DBG(
            "Processing field with unexposed type {}", cppast::to_string(tr));
        // TODO
    }

    if (!m.skip_relationship() &&
        !template_instantiation_added_as_aggregation &&
        (tr.kind() != cppast::cpp_type_kind::builtin_t) &&
        (tr.kind() != cppast::cpp_type_kind::template_parameter_t)) {
        const auto &ttt = resolve_alias(mv.type());
        std::vector<std::pair<std::string, relationship_t>> relationships;
        auto found = find_relationships(ttt, relationships);

        for (const auto &[type, relationship_type] : relationships) {
            if (relationship_type != relationship_t::kNone) {
                class_relationship r;
                r.destination = type;
                r.type = relationship_type;
                r.label = m.name();
                r.scope = m.scope();
                r.set_style(m.style_spec());

                auto [decorator_rtype, decorator_rmult] = m.relationship();
                if (decorator_rtype != relationship_t::kNone) {
                    r.type = decorator_rtype;
                    auto mult = util::split(decorator_rmult, ":");
                    if (mult.size() == 2) {
                        r.multiplicity_source = mult[0];
                        r.multiplicity_destination = mult[1];
                    }
                }

                LOG_DBG("Adding field relationship {} {} {} : {}",
                    r.destination, model::class_diagram::to_string(r.type),
                    c.full_name(), r.label);

                c.add_relationship(std::move(r));
            }
        }
    }

    c.add_member(std::move(m));
}

void tu_visitor::process_anonymous_enum(
    const cppast::cpp_enum &en, class_ &c, cppast::cpp_access_specifier_kind as)
{
    for (const auto &ev : en) {
        if (ev.kind() == cppast::cpp_entity_kind::enum_value_t) {
            class_member m{
                detail::cpp_access_specifier_to_scope(as), ev.name(), "enum"};
            c.add_member(std::move(m));
        }
    }
}

void tu_visitor::process_static_field(const cppast::cpp_variable &mv, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_member m{detail::cpp_access_specifier_to_scope(as), mv.name(),
        cppast::to_string(mv.type())};

    m.is_static(true);

    if (mv.comment().has_value())
        m.add_decorators(decorators::parse(mv.comment().value()));

    if (m.skip())
        return;

    c.add_member(std::move(m));
}

void tu_visitor::process_method(const cppast::cpp_member_function &mf,
    class_ &c, cppast::cpp_access_specifier_kind as)
{
    class_method m{detail::cpp_access_specifier_to_scope(as),
        util::trim(mf.name()), cppast::to_string(mf.return_type())};
    m.is_pure_virtual(cppast::is_pure(mf.virtual_info()));
    m.is_virtual(cppast::is_virtual(mf.virtual_info()));
    m.is_const(cppast::is_const(mf.cv_qualifier()));
    m.is_defaulted(false);
    m.is_static(false);

    if (mf.comment().has_value())
        m.add_decorators(decorators::parse(mf.comment().value()));

    if (m.skip())
        return;

    for (auto &param : mf.parameters())
        process_function_parameter(param, m, c);

    LOG_DBG("Adding method: {}", m.name());

    c.add_method(std::move(m));
}

void tu_visitor::process_template_method(
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

    class_method m{
        detail::cpp_access_specifier_to_scope(as), util::trim(mf.name()), type};
    m.is_pure_virtual(false);
    m.is_virtual(false);
    m.is_const(cppast::is_const(
        static_cast<const cppast::cpp_member_function &>(mf.function())
            .cv_qualifier()));
    m.is_defaulted(false);
    m.is_static(false);

    if (mf.comment().has_value())
        m.add_decorators(decorators::parse(mf.comment().value()));

    if (m.skip())
        return;

    std::set<std::string> template_parameter_names;
    for (const auto &template_parameter : mf.parameters()) {
        template_parameter_names.emplace(template_parameter.name());
    }

    for (auto &param : mf.function().parameters())
        process_function_parameter(param, m, c, template_parameter_names);

    LOG_DBG("Adding template method: {}", m.name());

    c.add_method(std::move(m));
}

void tu_visitor::process_static_method(const cppast::cpp_function &mf,
    class_ &c, cppast::cpp_access_specifier_kind as)
{
    class_method m{detail::cpp_access_specifier_to_scope(as),
        util::trim(mf.name()), cppast::to_string(mf.return_type())};
    m.is_pure_virtual(false);
    m.is_virtual(false);
    m.is_const(false);
    m.is_defaulted(false);
    m.is_static(true);

    if (mf.comment().has_value())
        m.add_decorators(decorators::parse(mf.comment().value()));

    if (m.skip())
        return;

    for (auto &param : mf.parameters())
        process_function_parameter(param, m, c);

    LOG_DBG("Adding static method: {}", m.name());

    c.add_method(std::move(m));
}

void tu_visitor::process_constructor(const cppast::cpp_constructor &mf,
    class_ &c, cppast::cpp_access_specifier_kind as)
{
    class_method m{detail::cpp_access_specifier_to_scope(as),
        util::trim(mf.name()), "void"};
    m.is_pure_virtual(false);
    m.is_virtual(false);
    m.is_const(false);
    m.is_defaulted(false);
    m.is_static(true);

    if (mf.comment().has_value())
        m.add_decorators(decorators::parse(mf.comment().value()));

    if (m.skip())
        return;

    for (auto &param : mf.parameters())
        process_function_parameter(param, m, c);

    c.add_method(std::move(m));
}

void tu_visitor::process_destructor(const cppast::cpp_destructor &mf, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_method m{detail::cpp_access_specifier_to_scope(as),
        util::trim(mf.name()), "void"};
    m.is_pure_virtual(false);
    m.is_virtual(cppast::is_virtual(mf.virtual_info()));
    m.is_const(false);
    m.is_defaulted(false);
    m.is_static(true);

    c.add_method(std::move(m));
}

void tu_visitor::process_function_parameter(
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
            if ((relationship_type != relationship_t::kNone) &&
                (type != c.name())) {
                class_relationship r;
                r.destination = type;
                r.type = relationship_t::kDependency;
                r.label = "";

                LOG_DBG("Adding field relationship {} {} {} : {}",
                    r.destination, model::class_diagram::to_string(r.type),
                    c.full_name(), r.label);

                c.add_relationship(std::move(r));
            }
        }

        // Also consider the container itself if it is user defined type
        const auto &t = cppast::remove_cv(cx::util::unreferenced(param.type()));
        if (t.kind() == cppast::cpp_type_kind::template_instantiation_t) {
            auto &template_instantiation_type =
                static_cast<const cppast::cpp_template_instantiation_type &>(t);

            if (template_instantiation_type.primary_template()
                    .get(ctx.entity_index)
                    .size()) {

                // Here we need the name of the primary template with full
                // namespace prefix to apply config inclusion filters
                auto primary_template_name = cx::util::full_name(ctx.namespace_,
                    template_instantiation_type.primary_template()
                        .get(ctx.entity_index)[0]
                        .get());
                // Now check if the template arguments of this function param
                // are a subset of the method template params - if yes this is
                // not an instantiation but just a reference to an existing
                // template
                bool template_is_not_instantiation{false};
                for (const auto &template_argument :
                    template_instantiation_type.arguments().value()) {
                    const auto template_argument_name =
                        cppast::to_string(template_argument.type().value());
                    if (template_parameter_names.count(template_argument_name) >
                        0) {
                        template_is_not_instantiation = true;
                        break;
                    }
                }

                LOG_DBG("Maybe building instantiation for: {}",
                    primary_template_name);

                if (ctx.config.should_include(primary_template_name)) {

                    if (template_is_not_instantiation) {
                        LOG_DBG("Template is not an instantiation - "
                                "only adding reference to template {}",
                            cx::util::full_name(
                                cppast::remove_cv(t), ctx.entity_index, false));
                        class_relationship rr;
                        rr.destination = cx::util::full_name(
                            cppast::remove_cv(t), ctx.entity_index, false);
                        rr.type = relationship_t::kDependency;
                        rr.label = "";
                        LOG_DBG("Adding field template dependency relationship "
                                "{} {} {} "
                                ": {}",
                            rr.destination,
                            model::class_diagram::to_string(rr.type),
                            c.full_name(), rr.label);
                        c.add_relationship(std::move(rr));
                    }
                    else {
                        // First check if tinst already exists
                        class_ tinst = build_template_instantiation(
                            template_instantiation_type);

                        class_relationship rr;
                        rr.destination = tinst.full_name();
                        rr.type = relationship_t::kDependency;
                        rr.label = "";

                        LOG_DBG("Adding field dependency relationship {} {} {} "
                                ": {}",
                            rr.destination,
                            model::class_diagram::to_string(rr.type),
                            c.full_name(), rr.label);

                        c.add_relationship(std::move(rr));

                        ctx.d.add_class(std::move(tinst));
                    }
                }
            }
        }
    }

    m.add_parameter(std::move(mp));
}

void tu_visitor::process_template_type_parameter(
    const cppast::cpp_template_type_parameter &t, class_ &parent)
{
    class_template ct;
    ct.type = "";
    ct.default_value = "";
    ct.is_variadic = t.is_variadic();
    ct.name = t.name();
    if (ct.is_variadic)
        ct.name += "...";
    parent.add_template(std::move(ct));
}

void tu_visitor::process_template_nontype_parameter(
    const cppast::cpp_non_type_template_parameter &t, class_ &parent)
{
    class_template ct;
    ct.type = cppast::to_string(t.type());
    ct.name = t.name();
    ct.default_value = "";
    ct.is_variadic = t.is_variadic();
    if (ct.is_variadic)
        ct.name += "...";
    parent.add_template(std::move(ct));
}

void tu_visitor::process_template_template_parameter(
    const cppast::cpp_template_template_parameter &t, class_ &parent)
{
    class_template ct;
    ct.type = "";
    ct.name = t.name() + "<>";
    ct.default_value = "";
    parent.add_template(std::move(ct));
}

void tu_visitor::process_friend(const cppast::cpp_friend &f, class_ &parent)
{
    // Only process friends to other classes or class templates
    if (!f.entity() ||
        (f.entity().value().kind() != cppast::cpp_entity_kind::class_t) &&
            (f.entity().value().kind() !=
                cppast::cpp_entity_kind::class_template_t))
        return;

    class_relationship r;
    r.type = relationship_t::kFriendship;
    r.label = "<<friend>>";

    if (f.comment().has_value())
        r.add_decorators(decorators::parse(f.comment().value()));

    if (r.skip() || r.skip_relationship())
        return;

    if (f.type()) {
        auto name = cppast::to_string(f.type().value());

        if (!ctx.config.should_include(name))
            return;

        LOG_DBG("Type friend declaration {}", name);

        r.destination = name;
    }
    else if (f.entity()) {
        std::string name{};

        if (f.entity().value().kind() ==
            cppast::cpp_entity_kind::class_template_t) {
            const auto &ft = static_cast<const cppast::cpp_class_template &>(
                f.entity().value());
            const auto &class_ = ft.class_();
            auto scope = cppast::cpp_scope_name(type_safe::ref(ft));
            if (ft.class_().user_data() == nullptr) {
                spdlog::warn(
                    "Empty user data in friend class template: {}, {}, {}",
                    ft.name(),
                    fmt::ptr(reinterpret_cast<const void *>(&ft.class_())),
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

            name = cx::util::full_name(ctx.namespace_, f.entity().value());
        }

        if (!ctx.config.should_include(name))
            return;

        r.destination = name;
    }
    else {
        LOG_DBG("Friend declaration points neither to type or entity.");
        return;
    }

    parent.add_relationship(std::move(r));
}

bool tu_visitor::find_relationships(const cppast::cpp_type &t_,
    std::vector<std::pair<std::string, relationship_t>> &relationships,
    relationship_t relationship_hint)
{
    bool found{false};

    const auto fn =
        cx::util::full_name(cppast::remove_cv(t_), ctx.entity_index, false);

    LOG_DBG("Finding relationships for type {}, {}, {}", cppast::to_string(t_),
        t_.kind(), fn);

    relationship_t relationship_type = relationship_hint;
    const auto &t = cppast::remove_cv(cx::util::unreferenced(t_));

    if (t.kind() == cppast::cpp_type_kind::array_t) {
        auto &a = static_cast<const cppast::cpp_array_type &>(t);
        found = find_relationships(
            a.value_type(), relationships, relationship_t::kAggregation);
        return found;
    }

    auto name = cppast::to_string(t);

    if (t_.kind() == cppast::cpp_type_kind::pointer_t) {
        auto &p = static_cast<const cppast::cpp_pointer_type &>(t_);
        auto rt = relationship_t::kAssociation;
        if (relationship_hint == relationship_t::kDependency)
            rt = relationship_hint;
        found = find_relationships(p.pointee(), relationships, rt);
    }
    else if (t_.kind() == cppast::cpp_type_kind::reference_t) {
        auto &r = static_cast<const cppast::cpp_reference_type &>(t_);
        auto rt = relationship_t::kAssociation;
        if (r.reference_kind() == cppast::cpp_reference::cpp_ref_rvalue) {
            rt = relationship_t::kAggregation;
        }
        if (relationship_hint == relationship_t::kDependency)
            rt = relationship_hint;
        found = find_relationships(r.referee(), relationships, rt);
    }
    if (cppast::remove_cv(t_).kind() == cppast::cpp_type_kind::user_defined_t) {
        LOG_DBG("User defined type: {} | {}", cppast::to_string(t_),
            cppast::to_string(t_.canonical()));

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
            if (found)
                return found;
        }
    }
    else if (t.kind() == cppast::cpp_type_kind::template_instantiation_t) {
        class_relationship r;

        auto &tinst =
            static_cast<const cppast::cpp_template_instantiation_type &>(t);

        if (!tinst.arguments_exposed()) {
            LOG_DBG("Template instantiation {} has no exposed arguments", name);

            return found;
        }

        const auto &args = tinst.arguments().value();

        // Try to match common containers
        // TODO: Refactor to a separate class with configurable
        //       container list
        if (name.find("std::unique_ptr") == 0) {
            found = find_relationships(args[0u].type().value(), relationships,
                relationship_t::kAggregation);
        }
        else if (name.find("std::shared_ptr") == 0) {
            found = find_relationships(args[0u].type().value(), relationships,
                relationship_t::kAssociation);
        }
        else if (name.find("std::weak_ptr") == 0) {
            found = find_relationships(args[0u].type().value(), relationships,
                relationship_t::kAssociation);
        }
        else if (name.find("std::vector") == 0) {
            found = find_relationships(args[0u].type().value(), relationships,
                relationship_t::kAggregation);
        }
        else if (ctx.config.should_include(fn)) {
            LOG_DBG("User defined template instantiation: {} | {}",
                cppast::to_string(t_), cppast::to_string(t_.canonical()));

            if (relationship_type != relationship_t::kNone)
                relationships.emplace_back(
                    cppast::to_string(t), relationship_type);
            else
                relationships.emplace_back(
                    cppast::to_string(t), relationship_t::kAggregation);

            // Check if t_ has an alias in the alias index
            if (ctx.has_type_alias(fn)) {
                LOG_DBG("Find relationship in alias of {} | {}", fn,
                    cppast::to_string(ctx.get_type_alias(fn).get()));
                found = find_relationships(ctx.get_type_alias(fn).get(),
                    relationships, relationship_type);
                if (found)
                    return found;
            }

            return found;
        }
        else {
            for (const auto &arg : args) {
                if (arg.type()) {
                    found = find_relationships(
                        arg.type().value(), relationships, relationship_type);
                }
            }
        }
    }

    return found;
}

class_ tu_visitor::build_template_instantiation(
    const cppast::cpp_template_instantiation_type &t,
    std::optional<clanguml::model::class_diagram::class_ *> parent)
{
    class_ tinst{ctx.config.using_namespace};
    std::string full_template_name;

    std::deque<std::tuple<std::string, int, bool>> template_base_params{};

    // Determine the full template name
    if (t.primary_template().get(ctx.entity_index).size()) {
        const auto &primary_template_ref =
            static_cast<const cppast::cpp_class_template &>(
                t.primary_template().get(ctx.entity_index)[0].get())
                .class_();

        if (parent)
            LOG_DBG("Template parent is {}", (*parent)->full_name());
        else
            LOG_DBG("Template parent is empty");

        full_template_name =
            cx::util::full_name(ctx.namespace_, primary_template_ref);

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
            for (const auto &tp : primary_template_ref.scope_name()
                                      .value()
                                      .template_parameters()) {
                template_parameter_names.emplace_back(
                    tp.name(), tp.is_variadic());
            }
        }

        // Check if the primary template has any base classes
        int base_index = 0;
        for (const auto &base : primary_template_ref.bases()) {
            if (base.kind() == cppast::cpp_entity_kind::base_class_t) {
                const auto &base_class =
                    static_cast<const cppast::cpp_base_class &>(base);

                const auto base_class_name =
                    cppast::to_string(base_class.type());

                LOG_DBG("Found template instantiation base: {}, {}, {}",
                    cppast::to_string(base.kind()), base_class_name,
                    base_index);

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
            LOG_WARN("No user data for base template {}",
                primary_template_ref.name());
    }
    else {
        LOG_WARN("Template instantiation {} has no primary template",
            cppast::to_string(t));

        full_template_name = cppast::to_string(t);
    }

    LOG_DBG("Building template instantiation for {}", full_template_name);

    // Extract namespace from base template name
    std::vector<std::string> ns_toks;
    ns_toks = clanguml::util::split(
        full_template_name.substr(0, full_template_name.find('<')), "::");

    std::string ns;
    if (ns_toks.size() > 1) {
        ns = fmt::format(
            "{}::", fmt::join(ns_toks.begin(), ns_toks.end() - 1, "::"));
    }

    LOG_DBG("Template namespace is {}", ns);

    tinst.set_name(ns + util::split(cppast::to_string(t), "<")[0]);

    tinst.is_template_instantiation(true);

    if (tinst.full_name().substr(0, tinst.full_name().find('<')).find("::") ==
        std::string::npos) {
        tinst.set_name(ns + tinst.full_name());
    }

    // Process template argumetns
    int arg_index{0};
    bool variadic_params{false};
    for (const auto &targ : t.arguments().value()) {
        bool add_template_argument_as_base_class{false};
        class_template ct;
        if (targ.type()) {
            ct.type = cppast::to_string(targ.type().value());

            LOG_DBG("Template argument is a type {}", ct.type);
            auto fn = cx::util::full_name(
                cppast::remove_cv(cx::util::unreferenced(targ.type().value())),
                ctx.entity_index, false);

            if (targ.type().value().kind() ==
                cppast::cpp_type_kind::template_instantiation_t) {

                const auto &nested_template_parameter = static_cast<
                    const cppast::cpp_template_instantiation_type &>(
                    targ.type().value());

                class_relationship tinst_dependency;
                tinst_dependency.type = relationship_t::kDependency;
                tinst_dependency.label = "";

                std::string nnn{"empty"};
                if (parent)
                    nnn = (*parent)->name();

                class_ nested_tinst =
                    build_template_instantiation(nested_template_parameter,
                        ctx.config.should_include(tinst.full_name(false))
                            ? std::make_optional(&tinst)
                            : parent);

                tinst_dependency.destination = nested_tinst.full_name();

                auto nested_tinst_full_name = nested_tinst.full_name();

                if (ctx.config.should_include(fn)) {
                    ctx.d.add_class(std::move(nested_tinst));
                }

                if (ctx.config.should_include(tinst.full_name(false))) {
                    LOG_DBG("Creating nested template dependency to template "
                            "instantiation {}, {} -> {}",
                        fn, tinst.full_name(), tinst_dependency.destination);

                    tinst.add_relationship(std::move(tinst_dependency));
                }
                else if (parent) {
                    LOG_DBG("Creating nested template dependency to parent "
                            "template "
                            "instantiation {}, {} -> {}",
                        fn, (*parent)->full_name(),
                        tinst_dependency.destination);

                    (*parent)->add_relationship(std::move(tinst_dependency));
                }
                else {
                    LOG_DBG("No nested template dependency to template "
                            "instantiation: {}, {} -> {}",
                        fn, tinst.full_name(), tinst_dependency.destination);
                }
            }
            else if (targ.type().value().kind() ==
                cppast::cpp_type_kind::user_defined_t) {
                class_relationship tinst_dependency;
                tinst_dependency.type = relationship_t::kDependency;
                tinst_dependency.label = "";

                tinst_dependency.destination = cx::util::full_name(
                    cppast::remove_cv(
                        cx::util::unreferenced(targ.type().value())),
                    ctx.entity_index, false);

                LOG_DBG("Creating nested template dependency to user defined "
                        "type {} -> {}",
                    tinst.full_name(), tinst_dependency.destination);

                if (ctx.config.should_include(fn)) {
                    tinst.add_relationship(std::move(tinst_dependency));
                }
                else if (parent) {
                    (*parent)->add_relationship(std::move(tinst_dependency));
                }
            }
        }
        else if (targ.expression()) {
            const auto &exp = targ.expression().value();
            if (exp.kind() == cppast::cpp_expression_kind::literal_t)
                ct.type =
                    static_cast<const cppast::cpp_literal_expression &>(exp)
                        .value();
            else if (exp.kind() == cppast::cpp_expression_kind::unexposed_t)
                ct.type =
                    static_cast<const cppast::cpp_unexposed_expression &>(exp)
                        .expression()
                        .as_string();

            LOG_DBG("Template argument is an expression {}", ct.type);
        }

        // In case any of the template arguments are base classes, add
        // them as parents of the current template instantiation class
        if (template_base_params.size() > 0) {
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
                LOG_DBG("Adding template argument '{}' as base class", ct.type);

                class_parent cp;
                cp.access = class_parent::access_t::kPublic;
                cp.name = ct.type;

                tinst.add_parent(std::move(cp));
            }
        }

        LOG_DBG("Adding template argument '{}'", ct.type);

        tinst.add_template(std::move(ct));
    }

    // Add instantiation relationship to primary template of this
    // instantiation
    class_relationship r;
    const auto &tt = cppast::remove_cv(cx::util::unreferenced(t));
    auto fn = cx::util::full_name(tt, ctx.entity_index, false);
    fn = util::split(fn, "<")[0];

    if (ctx.has_type_alias(fn)) {
        // If this is a template alias - set the instantiation
        // relationship to the first alias target
        r.destination = cppast::to_string(ctx.get_type_alias(fn).get());
    }
    else {
        // Otherwise point to the base template
        r.destination = tinst.base_template();
    }
    r.type = relationship_t::kInstantiation;
    r.label = "";
    r.scope = scope_t::kNone;
    tinst.add_relationship(std::move(r));

    return tinst;
}

const cppast::cpp_type &tu_visitor::resolve_alias(const cppast::cpp_type &t)
{
    const auto &tt = cppast::remove_cv(cx::util::unreferenced(t));
    const auto fn = cx::util::full_name(tt, ctx.entity_index, false);
    if (ctx.has_type_alias(fn)) {
        return ctx.get_type_alias_final(tt).get();
    }

    return t;
}
}
}
}
