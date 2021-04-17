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
#include <cppast/cpp_template.hpp>
#include <cppast/cpp_type_alias.hpp>
#include <cppast/cpp_variable.hpp>
#include <spdlog/spdlog.h>

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

void tu_visitor::operator()(const cppast::cpp_entity &file)
{
    cppast::visit(file,
        [&, this](const cppast::cpp_entity &e, cppast::visitor_info info) {
            if (e.kind() == cppast::cpp_entity_kind::class_t) {
                LOG_DBG("========== Visiting '{}' - {}", cx::util::full_name(e),
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
                if (ctx.config.should_include(cx::util::fully_prefixed(cls)))
                    process_class_declaration(cls);
            }
            else if (e.kind() == cppast::cpp_entity_kind::enum_t) {
                LOG_DBG("========== Visiting '{}' - {}", cx::util::full_name(e),
                    cppast::to_string(e.kind()));

                auto &enm = static_cast<const cppast::cpp_enum &>(e);

                if (ctx.config.should_include(cx::util::fully_prefixed(enm)))
                    process_enum_declaration(enm);
            }
            else if (e.kind() == cppast::cpp_entity_kind::type_alias_t) {
                LOG_DBG("========== Visiting '{}' - {}", cx::util::full_name(e),
                    cppast::to_string(e.kind()));

                auto &ta = static_cast<const cppast::cpp_type_alias &>(e);
                type_alias t;
                t.alias = cx::util::full_name(ta);
                t.underlying_type = cx::util::full_name(ta.underlying_type(),
                    ctx.entity_index, cx::util::is_inside_class(e));

                ctx.add_type_alias(cx::util::full_name(ta),
                    type_safe::ref(ta.underlying_type()));

                ctx.d.add_type_alias(std::move(t));
            }
            else if (e.kind() == cppast::cpp_entity_kind::alias_template_t) {
                LOG_DBG("========== Visiting '{}' - {}", cx::util::full_name(e),
                    cppast::to_string(e.kind()));

                auto &at = static_cast<const cppast::cpp_alias_template &>(e);

                class_ tinst = build_template_instantiation(static_cast<
                    const cppast::cpp_template_instantiation_type &>(
                    at.type_alias().underlying_type()));

                class_relationship r;
                r.destination = tinst.base_template_usr;
                r.type = relationship_t::kInstantiation;
                r.label = "";
                tinst.add_relationship(std::move(r));

                LOG_DBG("Created template instantiation: {}, {}, {}",
                    tinst.name, tinst.usr, tinst.base_template_usr);

                ctx.d.add_class(std::move(tinst));
            }
        });
}

void tu_visitor::process_enum_declaration(const cppast::cpp_enum &enm)
{
    enum_ e;
    e.name = cx::util::full_name(enm);

    for (const auto &ev : enm) {
        if (ev.kind() == cppast::cpp_entity_kind::enum_value_t) {
            e.constants.push_back(ev.name());
        }
    }

    // Find if class is contained in another class
    for (auto cur = enm.parent(); cur; cur = cur.value().parent()) {
        // find nearest parent class, if any
        if (cur.value().kind() == cppast::cpp_entity_kind::class_t) {
            class_relationship containment;
            containment.type = relationship_t::kContainment;
            containment.destination = cx::util::full_name(cur.value());
            e.relationships.emplace_back(std::move(containment));

            LOG_DBG("Added relationship {} +-- {}", e.name,
                containment.destination);
            break;
        }
    }

    ctx.d.add_enum(std::move(e));
}

void tu_visitor::process_class_declaration(const cppast::cpp_class &cls)
{
    class_ c;
    c.is_struct = cls.class_kind() == cppast::cpp_class_kind::struct_t;
    c.name = cx::util::full_name(cls);

    cppast::cpp_access_specifier_kind last_access_specifier =
        cppast::cpp_access_specifier_kind::cpp_private;

    // Process class child entities
    if (c.is_struct)
        last_access_specifier = cppast::cpp_access_specifier_kind::cpp_public;

    for (auto &child : cls) {
        if (child.kind() == cppast::cpp_entity_kind::access_specifier_t) {
            auto &as = static_cast<const cppast::cpp_access_specifier &>(child);
            last_access_specifier = as.access_specifier();
        }
        else if (child.kind() == cppast::cpp_entity_kind::member_variable_t) {
            auto &mv = static_cast<const cppast::cpp_member_variable &>(child);
            LOG_DBG("Found member variable {}", mv.name());
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
        cp.name = clanguml::cx::util::fully_prefixed(base);
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

        c.bases.emplace_back(std::move(cp));
    }

    // Process class template arguments
    if (cppast::is_templated(cls)) {
        LOG_DBG("Processing class template parameters...");
        auto scope = cppast::cpp_scope_name(type_safe::ref(cls));
        for (const auto &tp : scope.template_parameters()) {
            if (tp.kind() ==
                cppast::cpp_entity_kind::template_type_parameter_t) {
                LOG_DBG("Processing template type parameter {}", tp.name());
                process_template_type_parameter(
                    static_cast<const cppast::cpp_template_type_parameter &>(
                        tp),
                    c);
            }
            else if (tp.kind() ==
                cppast::cpp_entity_kind::non_type_template_parameter_t) {
                LOG_DBG("Processing template nontype parameter {}", tp.name());
                process_template_nontype_parameter(
                    static_cast<
                        const cppast::cpp_non_type_template_parameter &>(tp),
                    c);
            }
            else if (tp.kind() ==
                cppast::cpp_entity_kind::template_template_parameter_t) {
                LOG_DBG("Processing template template parameter {}", tp.name());
                process_template_template_parameter(
                    static_cast<
                        const cppast::cpp_template_template_parameter &>(tp),
                    c);
            }
        }
    }

    // Find if class is contained in another class
    for (auto cur = cls.parent(); cur; cur = cur.value().parent()) {
        // find nearest parent class, if any
        if (cur.value().kind() == cppast::cpp_entity_kind::class_t) {
            class_relationship containment;
            containment.type = relationship_t::kContainment;
            containment.destination = cx::util::full_name(cur.value());
            c.add_relationship(std::move(containment));

            LOG_DBG("Added relationship {} +-- {}",
                c.full_name(ctx.config.using_namespace),
                containment.destination);

            break;
        }
    }

    cls.set_user_data(strdup(c.full_name(ctx.config.using_namespace).c_str()));

    LOG_DBG("Setting user data for class {}, {}",
        static_cast<const char *>(cls.user_data()),
        fmt::ptr(reinterpret_cast<const void *>(&cls)));

    c.usr = c.full_name(ctx.config.using_namespace);

    ctx.d.add_class(std::move(c));
}

void tu_visitor::process_field_with_template_instantiation(
    const cppast::cpp_member_variable &mv, const cppast::cpp_type &tr,
    class_ &c)
{
    LOG_DBG("Processing field with template instatiation type {}",
        cppast::to_string(tr));

    const auto &template_instantiation_type =
        static_cast<const cppast::cpp_template_instantiation_type &>(tr);
    if (template_instantiation_type.primary_template()
            .get(ctx.entity_index)
            .size()) {
        // Here we need the name of the primary template with full namespace
        // prefix to apply config inclusion filters
        auto primary_template_name =
            cx::util::full_name(template_instantiation_type.primary_template()
                                    .get(ctx.entity_index)[0]
                                    .get());

        LOG_DBG("Maybe building instantiation for: {}{}", primary_template_name,
            cppast::to_string(tr));

        if (ctx.config.should_include(primary_template_name)) {
            const auto &unaliased =
                static_cast<const cppast::cpp_template_instantiation_type &>(
                    resolve_alias(template_instantiation_type));
            class_ tinst = build_template_instantiation(unaliased);

            class_relationship r;
            const auto &tt = cppast::remove_cv(
                cx::util::unreferenced(template_instantiation_type));
            auto fn = cx::util::full_name(tt, ctx.entity_index, false);
            fn = util::split(fn, "<")[0];

            if (ctx.has_type_alias(fn)) {
                // If this is a template alias - set the instantiation
                // relationship to the first alias target
                r.destination = cppast::to_string(ctx.get_type_alias(fn).get());
            }
            else {
                // Otherwise point to the base template
                r.destination = tinst.base_template_usr;
            }
            r.type = relationship_t::kInstantiation;
            r.label = "";
            tinst.add_relationship(std::move(r));

            class_relationship rr;
            rr.destination = tinst.usr;
            if (mv.type().kind() == cppast::cpp_type_kind::pointer_t ||
                mv.type().kind() == cppast::cpp_type_kind::reference_t)
                rr.type = relationship_t::kAssociation;
            else
                rr.type = relationship_t::kComposition;
            rr.label = mv.name();
            LOG_DBG("Adding field instantiation relationship {} {} {} : {}",
                rr.destination, model::class_diagram::to_string(rr.type), c.usr,
                rr.label);
            c.add_relationship(std::move(rr));

            LOG_DBG("Created template instantiation: {}, {}", tinst.name,
                tinst.usr);

            ctx.d.add_class(std::move(tinst));
        }
    }
}

void tu_visitor::process_field(const cppast::cpp_member_variable &mv, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_member m;
    m.name = mv.name();
    m.type = cppast::to_string(mv.type());
    m.scope = detail::cpp_access_specifier_to_scope(as);
    m.is_static = false;

    auto &tr = cx::util::unreferenced(cppast::remove_cv(mv.type()));

    LOG_DBG("Processing field {} with unreferenced type of kind {}", mv.name(),
        tr.kind());

    if (tr.kind() == cppast::cpp_type_kind::builtin_t) {
        LOG_DBG("Builtin type found for field: {}", m.name);
    }
    else if (tr.kind() == cppast::cpp_type_kind::user_defined_t) {
        LOG_DBG("Processing user defined type field {} {}",
            cppast::to_string(tr), mv.name());
    }
    else if (tr.kind() == cppast::cpp_type_kind::template_instantiation_t) {
        process_field_with_template_instantiation(mv, resolve_alias(tr), c);
    }
    else if (tr.kind() == cppast::cpp_type_kind::unexposed_t) {
        LOG_DBG(
            "Processing field with unexposed type {}", cppast::to_string(tr));
        // TODO
    }

    if (tr.kind() != cppast::cpp_type_kind::builtin_t &&
        tr.kind() != cppast::cpp_type_kind::template_parameter_t) {
        const auto &ttt = resolve_alias(mv.type());
        std::vector<std::pair<std::string, relationship_t>> relationships;
        find_relationships(ttt, relationships);

        for (const auto &[type, relationship_type] : relationships) {
            if (relationship_type != relationship_t::kNone) {
                class_relationship r;
                r.destination = type;
                r.type = relationship_type;
                r.label = m.name;

                LOG_DBG("Adding field relationship {} {} {} : {}",
                    r.destination, model::class_diagram::to_string(r.type),
                    c.usr, r.label);

                c.relationships.emplace_back(std::move(r));
            }
        }
    }

    c.members.emplace_back(std::move(m));
}

void tu_visitor::process_static_field(const cppast::cpp_variable &mv, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_member m;
    m.name = mv.name();
    m.type = cppast::to_string(mv.type());
    m.scope = detail::cpp_access_specifier_to_scope(as);
    m.is_static = true;

    c.members.emplace_back(std::move(m));
}

void tu_visitor::process_method(const cppast::cpp_member_function &mf,
    class_ &c, cppast::cpp_access_specifier_kind as)
{
    class_method m;
    m.name = util::trim(mf.name());
    m.type = cppast::to_string(mf.return_type());
    m.is_pure_virtual = cppast::is_pure(mf.virtual_info());
    m.is_virtual = cppast::is_virtual(mf.virtual_info());
    m.is_const = cppast::is_const(mf.cv_qualifier());
    m.is_defaulted = false;
    m.is_static = false;
    m.scope = detail::cpp_access_specifier_to_scope(as);

    for (auto &param : mf.parameters())
        process_function_parameter(param, m, c);

    LOG_DBG("Adding method: {}", m.name);

    c.methods.emplace_back(std::move(m));
}

void tu_visitor::process_template_method(
    const cppast::cpp_function_template &mf, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_method m;
    m.name = util::trim(mf.name());
    m.type = cppast::to_string(
        static_cast<const cppast::cpp_member_function &>(mf.function())
            .return_type());
    m.is_pure_virtual = false;
    m.is_virtual = false;
    m.is_const = cppast::is_const(
        static_cast<const cppast::cpp_member_function &>(mf.function())
            .cv_qualifier());
    m.is_defaulted = false;
    m.is_static = false;
    m.scope = detail::cpp_access_specifier_to_scope(as);

    for (auto &param : mf.function().parameters())
        process_function_parameter(param, m, c);

    LOG_DBG("Adding template method: {}", m.name);

    c.methods.emplace_back(std::move(m));
}

void tu_visitor::process_static_method(const cppast::cpp_function &mf,
    class_ &c, cppast::cpp_access_specifier_kind as)
{
    class_method m;
    m.name = util::trim(mf.name());
    m.type = cppast::to_string(mf.return_type());
    m.is_pure_virtual = false;
    m.is_virtual = false;
    m.is_const = false;
    m.is_defaulted = false;
    m.is_static = true;
    m.scope = detail::cpp_access_specifier_to_scope(as);

    for (auto &param : mf.parameters())
        process_function_parameter(param, m, c);

    LOG_DBG("Adding static method: {}", m.name);

    c.methods.emplace_back(std::move(m));
}

void tu_visitor::process_constructor(const cppast::cpp_constructor &mf,
    class_ &c, cppast::cpp_access_specifier_kind as)
{
    class_method m;
    m.name = util::trim(mf.name());
    m.type = "void";
    m.is_pure_virtual = false;
    m.is_virtual = false;
    m.is_const = false;
    m.is_defaulted = false;
    m.is_static = false;
    m.scope = detail::cpp_access_specifier_to_scope(as);

    for (auto &param : mf.parameters())
        process_function_parameter(param, m, c);

    c.methods.emplace_back(std::move(m));
}

void tu_visitor::process_destructor(const cppast::cpp_destructor &mf, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_method m;
    m.name = util::trim(mf.name());
    m.type = "void";
    m.is_pure_virtual = false;
    m.is_virtual = cppast::is_virtual(mf.virtual_info());
    m.is_const = false;
    m.is_defaulted = false;
    m.is_static = false;
    m.scope = detail::cpp_access_specifier_to_scope(as);

    c.methods.emplace_back(std::move(m));
}

void tu_visitor::process_function_parameter(
    const cppast::cpp_function_parameter &param, class_method &m, class_ &c)
{
    method_parameter mp;
    mp.name = param.name();
    mp.type = cppast::to_string(param.type());

    auto dv = param.default_value();
    if (dv)
        switch (dv.value().kind()) {
        case cppast::cpp_expression_kind::literal_t:
            mp.default_value =
                static_cast<const cppast::cpp_literal_expression &>(dv.value())
                    .value();
            break;
        case cppast::cpp_expression_kind::unexposed_t:
            mp.default_value =
                static_cast<const cppast::cpp_unexposed_expression &>(
                    dv.value())
                    .expression()
                    .as_string();
            break;
        default:
            mp.default_value = "{}";
        }

    // find relationship for the type
    std::vector<std::pair<std::string, relationship_t>> relationships;
    find_relationships(cppast::remove_cv(param.type()), relationships,
        relationship_t::kDependency);
    for (const auto &[type, relationship_type] : relationships) {
        if ((relationship_type != relationship_t::kNone) && (type != c.name)) {
            class_relationship r;
            r.destination = type;
            r.type = relationship_t::kDependency;
            r.label = "";

            LOG_DBG("Adding field relationship {} {} {} : {}", r.destination,
                model::class_diagram::to_string(r.type), c.usr, r.label);

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
            auto primary_template_name = cx::util::full_name(
                template_instantiation_type.primary_template()
                    .get(ctx.entity_index)[0]
                    .get());

            LOG_DBG(
                "Maybe building instantiation for: {}", primary_template_name);

            if (ctx.config.should_include(primary_template_name)) {
                class_ tinst =
                    build_template_instantiation(template_instantiation_type);

                LOG_DBG("Created template instantiation: {}, {}", tinst.name,
                    tinst.usr);

                class_relationship r;
                r.destination = tinst.base_template_usr;
                r.type = relationship_t::kInstantiation;
                r.label = "";
                tinst.add_relationship(std::move(r));

                class_relationship rr;
                rr.destination = tinst.usr;
                rr.type = relationship_t::kDependency;
                rr.label = "";
                LOG_DBG("Adding field instantiation relationship {} {} {} : {}",
                    rr.destination, model::class_diagram::to_string(rr.type),
                    c.usr, rr.label);
                c.add_relationship(std::move(rr));

                ctx.d.add_class(std::move(tinst));
            }
        }
    }

    m.parameters.emplace_back(std::move(mp));
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
    parent.templates.emplace_back(std::move(ct));
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
    parent.templates.emplace_back(std::move(ct));
}

void tu_visitor::process_template_template_parameter(
    const cppast::cpp_template_template_parameter &t, class_ &parent)
{
    class_template ct;
    ct.type = "";
    ct.name = t.name() + "<>";
    ct.default_value = "";
    parent.templates.emplace_back(std::move(ct));
}

void tu_visitor::process_friend(const cppast::cpp_friend &f, class_ &parent)
{
    class_relationship r;
    r.type = relationship_t::kFriendship;
    r.label = "<<friend>>";

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

            name = cx::util::full_name(f.entity().value());
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

void tu_visitor::find_relationships(const cppast::cpp_type &t_,
    std::vector<std::pair<std::string, relationship_t>> &relationships,
    relationship_t relationship_hint)
{
    LOG_DBG("Finding relationships for type {}, {}", cppast::to_string(t_),
        t_.kind());

    relationship_t relationship_type = relationship_hint;
    const auto &t = cppast::remove_cv(cx::util::unreferenced(t_));

    if (t.kind() == cppast::cpp_type_kind::array_t) {
        auto &a = static_cast<const cppast::cpp_array_type &>(t);
        find_relationships(
            a.value_type(), relationships, relationship_t::kComposition);
        return;
    }

    auto name = cppast::to_string(t);

    if (t.kind() == cppast::cpp_type_kind::template_instantiation_t) {
        class_relationship r;

        auto &tinst =
            static_cast<const cppast::cpp_template_instantiation_type &>(t);

        if (!tinst.arguments_exposed()) {
            LOG_DBG("Template instantiation {} has no exposed arguments", name);

            return;
        }

        const auto &args = tinst.arguments().value();

        // Try to match common containers
        // TODO: Refactor to a separate class with configurable
        //       container list
        if (name.find("std::unique_ptr") == 0) {
            find_relationships(args[0u].type().value(), relationships,
                relationship_t::kComposition);
        }
        else if (name.find("std::shared_ptr") == 0) {
            find_relationships(args[0u].type().value(), relationships,
                relationship_t::kAssociation);
        }
        else if (name.find("std::weak_ptr") == 0) {
            find_relationships(args[0u].type().value(), relationships,
                relationship_t::kAssociation);
        }
        else if (name.find("std::vector") == 0) {
            find_relationships(args[0u].type().value(), relationships,
                relationship_t::kComposition);
        }
        else {
            for (const auto &arg : args) {
                if (arg.type())
                    find_relationships(
                        arg.type().value(), relationships, relationship_type);
            }
        }
    }
    else if (t_.kind() == cppast::cpp_type_kind::pointer_t) {
        auto &p = static_cast<const cppast::cpp_pointer_type &>(t_);
        auto rt = relationship_t::kAssociation;
        if (relationship_hint == relationship_t::kDependency)
            rt = relationship_hint;
        find_relationships(p.pointee(), relationships, rt);
    }
    else if (t_.kind() == cppast::cpp_type_kind::reference_t) {
        auto &r = static_cast<const cppast::cpp_reference_type &>(t_);
        auto rt = relationship_t::kAssociation;
        if (r.reference_kind() == cppast::cpp_reference::cpp_ref_rvalue) {
            rt = relationship_t::kComposition;
        }
        if (relationship_hint == relationship_t::kDependency)
            rt = relationship_hint;
        find_relationships(r.referee(), relationships, rt);
    }
    else if (cppast::remove_cv(t_).kind() ==
        cppast::cpp_type_kind::user_defined_t) {
        LOG_DBG("User defined type: {} | {}", cppast::to_string(t_),
            cppast::to_string(t_.canonical()));
        if (ctx.config.should_include(cppast::to_string(t_.canonical()))) {
            if (relationship_type != relationship_t::kNone)
                relationships.emplace_back(
                    cppast::to_string(t), relationship_type);
            else
                relationships.emplace_back(
                    cppast::to_string(t), relationship_t::kComposition);
        }

        // Check if t_ has an alias in the alias index
        const auto fn =
            cx::util::full_name(cppast::remove_cv(t_), ctx.entity_index, false);
        if (ctx.has_type_alias(fn)) {
            LOG_DBG("Find relationship in alias of {} | {}", fn,
                cppast::to_string(ctx.get_type_alias(fn).get()));
            find_relationships(
                ctx.get_type_alias(fn).get(), relationships, relationship_type);
        }
    }
}

class_ tu_visitor::build_template_instantiation(/*const cppast::cpp_entity &e,*/
    const cppast::cpp_template_instantiation_type &t)
{
    auto full_template_name = cx::util::full_name(
        t.primary_template().get(ctx.entity_index)[0].get());

    LOG_DBG("Found template instantiation: {} ({}) ..|> {}, {}",
        cppast::to_string(t), cppast::to_string(t.canonical()),
        t.primary_template().name(), full_template_name);

    class_ tinst;
    const auto &primary_template_ref =
        static_cast<const cppast::cpp_class_template &>(
            t.primary_template().get(ctx.entity_index)[0].get())
            .class_();

    tinst.name = util::split(
        cppast::to_string(t), "<")[0]; // primary_template_ref.name();
    if (full_template_name.back() == ':')
        tinst.name = full_template_name + tinst.name;

    if (primary_template_ref.user_data()) {
        tinst.base_template_usr =
            static_cast<const char *>(primary_template_ref.user_data());
        LOG_DBG("Primary template ref set to: {}", tinst.base_template_usr);
    }
    else
        LOG_WARN(
            "No user data for base template {}", primary_template_ref.name());

    tinst.is_template_instantiation = true;

    for (const auto &targ : t.arguments().value()) {
        class_template ct;
        if (targ.type()) {
            ct.type = cppast::to_string(targ.type().value());
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
        }

        LOG_DBG("Adding template argument '{}'", ct.type);

        tinst.templates.emplace_back(std::move(ct));
    }

    tinst.usr = tinst.full_name(ctx.config.using_namespace);

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
