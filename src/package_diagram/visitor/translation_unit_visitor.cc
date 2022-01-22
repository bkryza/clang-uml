/**
 * src/package_diagram/visitor/translation_unit_visitor.cc
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

namespace clanguml::package_diagram::visitor {

using clanguml::class_diagram::model::type_alias;
using clanguml::common::model::access_t;
using clanguml::common::model::relationship;
using clanguml::common::model::relationship_t;
using clanguml::common::model::scope_t;
using clanguml::package_diagram::model::diagram;
using clanguml::package_diagram::model::package;

namespace detail {
scope_t cpp_access_specifier_to_scope(
    cppast::cpp_access_specifier_kind access_specifier)
{
    scope_t scope = scope_t::kPublic;
    switch (access_specifier) {
    case cppast::cpp_access_specifier_kind::cpp_public:
        scope = scope_t::kPublic;
        break;
    case cppast::cpp_access_specifier_kind::cpp_private:
        scope = scope_t::kPrivate;
        break;
    case cppast::cpp_access_specifier_kind::cpp_protected:
        scope = scope_t::kProtected;
        break;
    default:
        break;
    }

    return scope;
}
}

translation_unit_visitor::translation_unit_visitor(
    cppast::cpp_entity_index &idx,
    clanguml::package_diagram::model::diagram &diagram,
    const clanguml::config::package_diagram &config)
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

                        auto package_path = ctx.get_namespace();
                        package_path.push_back(e.name());

                        auto usn =
                            util::split(ctx.config().using_namespace[0], "::");

                        if (!starts_with(usn, package_path)) {
                            auto p = std::make_unique<package>(
                                ctx.config().using_namespace);
                            remove_prefix(package_path, usn);
                            package_path.pop_back();

                            p->set_name(e.name());
                            p->set_namespace(package_path);
                            ctx.diagram().add_package(
                                package_path, std::move(p));
                        }

                        ctx.push_namespace(e.name());
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
                    auto &clsdef = static_cast<const cppast::cpp_class &>(
                        cppast::get_definition(ctx.entity_index(), cls)
                            .value());
                    if (&cls != &clsdef) {
                        LOG_DBG("Forward declaration of class {} - skipping...",
                            cls.name());
                        return;
                    }
                }

                if (ctx.config().should_include(
                        cx::util::fully_prefixed(ctx.get_namespace(), cls)))
                    process_class_declaration(cls);
            }
            //            else if (e.kind() == cppast::cpp_entity_kind::enum_t)
            //            {
            //                LOG_DBG("========== Visiting '{}' - {}",
            //                    cx::util::full_name(ctx.get_namespace(), e),
            //                    cppast::to_string(e.kind()));
            //
            //                auto &enm = static_cast<const cppast::cpp_enum
            //                &>(e);
            //
            //                if (ctx.config().should_include(
            //                        cx::util::fully_prefixed(ctx.get_namespace(),
            //                        enm)))
            //                    process_enum_declaration(enm);
            //            }
            else if (e.kind() == cppast::cpp_entity_kind::type_alias_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.get_namespace(), e),
                    cppast::to_string(e.kind()));

                auto &ta = static_cast<const cppast::cpp_type_alias &>(e);
                type_alias t;
                t.set_alias(cx::util::full_name(ctx.get_namespace(), ta));
                t.set_underlying_type(cx::util::full_name(ta.underlying_type(),
                    ctx.entity_index(), cx::util::is_inside_class(e)));

                ctx.add_type_alias(cx::util::full_name(ctx.get_namespace(), ta),
                    type_safe::ref(ta.underlying_type()));

                // ctx.diagram().add_type_alias(std::move(t));
            }
            else if (e.kind() == cppast::cpp_entity_kind::alias_template_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.get_namespace(), e),
                    cppast::to_string(e.kind()));

                auto &at = static_cast<const cppast::cpp_alias_template &>(e);

                //                if (at.type_alias().underlying_type().kind()
                //                ==
                //                    cppast::cpp_type_kind::unexposed_t) {
                //                    LOG_WARN("Template alias has unexposed
                //                    underlying type: {}",
                //                        static_cast<const
                //                        cppast::cpp_unexposed_type &>(
                //                            at.type_alias().underlying_type())
                //                            .name());
                //                }
                //                else {
                //                    class_ tinst =
                //                    build_template_instantiation(static_cast<
                //                        const
                //                        cppast::cpp_template_instantiation_type
                //                        &>(
                //                        at.type_alias().underlying_type()));
                //
                //                    ctx.diagram().add_class(std::move(tinst));
                //                }
            }
        });
}

void translation_unit_visitor::process_class_declaration(
    const cppast::cpp_class &cls,
    type_safe::optional_ref<const cppast::cpp_template_specialization> tspec)
{

    return;
    /*
    class_ c{ctx.config().using_namespace};
    c.is_struct(cls.class_kind() == cppast::cpp_class_kind::struct_t);
    c.set_name(cx::util::full_name(ctx.get_namespace(), cls));

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
        cp.set_name(
            clanguml::cx::util::fully_prefixed(ctx.get_namespace(), base));
        cp.is_virtual(base.is_virtual());

        switch (base.access_specifier()) {
        case cppast::cpp_access_specifier_kind::cpp_private:
            cp.set_access(access_t::kPrivate);
            break;
        case cppast::cpp_access_specifier_kind::cpp_public:
            cp.set_access(access_t::kPublic);
            break;
        case cppast::cpp_access_specifier_kind::cpp_protected:
            cp.set_access(access_t::kProtected);
            break;
        default:
            cp.set_access(access_t::kPublic);
        }

        LOG_DBG("Found base class {} for class {}", cp.name(), c.name());

        c.add_parent(std::move(cp));
    }

    ctx.diagram().add_class(std::move(c));
     */
}

const cppast::cpp_type &translation_unit_visitor::resolve_alias(
    const cppast::cpp_type &type)
{
    const auto &raw_type = cppast::remove_cv(cx::util::unreferenced(type));
    const auto type_full_name =
        cx::util::full_name(raw_type, ctx.entity_index(), false);
    if (ctx.has_type_alias(type_full_name)) {
        return ctx.get_type_alias_final(raw_type).get();
    }

    return type;
}
}
