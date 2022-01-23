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

                        std::vector<std::string> package_parent =
                            ctx.get_namespace();
                        auto package_path = package_parent;
                        package_path.push_back(e.name());

                        auto usn =
                            util::split(ctx.config().using_namespace[0], "::");

                        if (!starts_with(usn, package_path)) {
                            auto p = std::make_unique<package>(
                                ctx.config().using_namespace);
                            remove_prefix(package_path, usn);
                            remove_prefix(package_parent, usn);

                            p->set_name(e.name());
                            p->set_namespace(package_parent);

                            ctx.diagram().add_package(
                                package_parent, std::move(p));
                            ctx.set_current_package(
                                ctx.diagram().get_package(package_path));
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
            }
        });
}

void translation_unit_visitor::process_class_declaration(
    const cppast::cpp_class &cls,
    type_safe::optional_ref<const cppast::cpp_template_specialization> tspec)
{
    auto current_package = ctx.get_current_package();

    if (!current_package)
        return;

    cppast::cpp_access_specifier_kind last_access_specifier =
        cppast::cpp_access_specifier_kind::cpp_private;

    for (auto &child : cls) {
        if (child.kind() == cppast::cpp_entity_kind::access_specifier_t) {
            auto &as = static_cast<const cppast::cpp_access_specifier &>(child);
            last_access_specifier = as.access_specifier();
        }
        else if (child.kind() == cppast::cpp_entity_kind::member_variable_t) {
            auto &mv = static_cast<const cppast::cpp_member_variable &>(child);
            process_field(mv, current_package, last_access_specifier);
        }
        else {
            LOG_DBG("Found some other class child: {} ({})", child.name(),
                cppast::to_string(child.kind()));
        }
    }
}

void translation_unit_visitor::process_field(
    const cppast::cpp_member_variable &mv,
    type_safe::optional_ref<model::package> p,
    cppast::cpp_access_specifier_kind as)
{
    auto &type = cx::util::unreferenced(cppast::remove_cv(mv.type()));
    auto type_ns =
        util::split(cx::util::full_name(type, ctx.entity_index(), false), "::");
    type_ns.pop_back();

    if (type.kind() == cppast::cpp_type_kind::user_defined_t) {
        LOG_DBG("Processing user defined type field {} {}",
            cppast::to_string(type), mv.name());

        if (!starts_with(ctx.get_namespace(), type_ns) &&
            !starts_with(type_ns, ctx.get_namespace())) {
            relationship r{relationship_t::kDependency,
                fmt::format("{}", fmt::join(type_ns, "::"))};
            p.value().add_relationship(std::move(r));
        }
    }
    else if (type.kind() == cppast::cpp_type_kind::template_instantiation_t) {
        //        template_instantiation_added_as_aggregation =
        //            process_field_with_template_instantiation(
        //                mv, resolve_alias(type), c, m, as);
        LOG_DBG("Processing template instantiation type {} {}",
            cppast::to_string(type), mv.name());
    }
    else {
        LOG_DBG("Skipping field type: {}", cppast::to_string(type));
    }
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
