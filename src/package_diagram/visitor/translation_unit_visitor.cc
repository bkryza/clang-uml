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

                            for (const auto &attr :
                                ns_declaration.attributes()) {
                                if (attr.kind() ==
                                    cppast::cpp_attribute_kind::deprecated) {
                                    p->set_deprecated(true);
                                    break;
                                }
                            }

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

                process_class_declaration(cls);
            }
            else if (e.kind() == cppast::cpp_entity_kind::function_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.get_namespace(), e),
                    cppast::to_string(e.kind()));

                auto &f = static_cast<const cppast::cpp_function &>(e);

                process_function(f);
            }
            else if (e.kind() == cppast::cpp_entity_kind::function_template_t) {
                LOG_DBG("========== Visiting '{}' - {}",
                    cx::util::full_name(ctx.get_namespace(), e),
                    cppast::to_string(e.kind()));

                auto &f = static_cast<const cppast::cpp_function &>(
                    static_cast<const cppast::cpp_function_template &>(e)
                        .function());

                process_function(f);
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

    std::vector<std::pair<std::string, relationship_t>> relationships;

    // Process class elements
    for (auto &child : cls) {
        if (child.kind() == cppast::cpp_entity_kind::access_specifier_t) {
            auto &as = static_cast<const cppast::cpp_access_specifier &>(child);
            last_access_specifier = as.access_specifier();
        }
        else if (child.kind() == cppast::cpp_entity_kind::member_variable_t) {
            auto &mv = static_cast<const cppast::cpp_member_variable &>(child);
            find_relationships(
                mv.type(), relationships, relationship_t::kDependency);
        }
        else if (child.kind() == cppast::cpp_entity_kind::variable_t) {
            auto &mv = static_cast<const cppast::cpp_variable &>(child);
            find_relationships(
                mv.type(), relationships, relationship_t::kDependency);
        }
        else if (child.kind() == cppast::cpp_entity_kind::member_function_t) {
            auto &mf = static_cast<const cppast::cpp_member_function &>(child);
            for (const auto &param : mf.parameters())
                find_relationships(
                    param.type(), relationships, relationship_t::kDependency);

            find_relationships(
                mf.return_type(), relationships, relationship_t::kDependency);
        }
        else if (child.kind() == cppast::cpp_entity_kind::function_t) {
            auto &mf = static_cast<const cppast::cpp_function &>(child);
            for (const auto &param : mf.parameters())
                find_relationships(
                    param.type(), relationships, relationship_t::kDependency);
        }
        else if (child.kind() == cppast::cpp_entity_kind::function_template_t) {
            auto &tm = static_cast<const cppast::cpp_function_template &>(child)
                           .function();
            for (const auto &param : tm.parameters())
                find_relationships(
                    param.type(), relationships, relationship_t::kDependency);

            if (tm.kind() == cppast::cpp_entity_kind::member_function_t)
                find_relationships(
                    static_cast<const cppast::cpp_member_function &>(tm)
                        .return_type(),
                    relationships, relationship_t::kDependency);
        }
        else if (child.kind() == cppast::cpp_entity_kind::constructor_t) {
            auto &mc = static_cast<const cppast::cpp_constructor &>(child);
            for (const auto &param : mc.parameters())
                find_relationships(
                    param.type(), relationships, relationship_t::kDependency);
        }
        else {
            LOG_DBG("Found some other class child: {} ({})", child.name(),
                cppast::to_string(child.kind()));
        }
    }

    // Process class bases
    for (auto &base : cls.bases()) {
        find_relationships(
            base.type(), relationships, relationship_t::kDependency);

        clanguml::cx::util::fully_prefixed(ctx.get_namespace(), base);
    }

    for (const auto &dependency : relationships) {
        auto type_ns = util::split(std::get<0>(dependency), "::");
        type_ns.pop_back();

        relationship r{relationship_t::kDependency,
            fmt::format("{}", fmt::join(type_ns, "::"))};

        if (!starts_with(ctx.get_namespace(), type_ns) &&
            !starts_with(type_ns, ctx.get_namespace())) {
            relationship r{relationship_t::kDependency,
                fmt::format("{}", fmt::join(type_ns, "::"))};
            current_package.value().add_relationship(std::move(r));
        }
    }
}

void translation_unit_visitor::process_function(const cppast::cpp_function &f)
{
    std::vector<std::pair<std::string, relationship_t>> relationships;
    auto current_package = ctx.get_current_package();

    if (!current_package)
        return;

    for (const auto &param : f.parameters())
        find_relationships(
            param.type(), relationships, relationship_t::kDependency);

    find_relationships(
        f.return_type(), relationships, relationship_t::kDependency);

    for (const auto &dependency : relationships) {
        auto type_ns = util::split(std::get<0>(dependency), "::");
        type_ns.pop_back();

        relationship r{relationship_t::kDependency,
            fmt::format("{}", fmt::join(type_ns, "::"))};

        if (!starts_with(ctx.get_namespace(), type_ns) &&
            !starts_with(type_ns, ctx.get_namespace())) {
            relationship r{relationship_t::kDependency,
                fmt::format("{}", fmt::join(type_ns, "::"))};
            current_package.value().add_relationship(std::move(r));
        }
    }
}

bool translation_unit_visitor::find_relationships(const cppast::cpp_type &t_,
    std::vector<std::pair<std::string, common::model::relationship_t>>
        &relationships,
    relationship_t relationship_hint)
{
    bool found{false};

    const auto fn =
        cx::util::full_name(cppast::remove_cv(t_), ctx.entity_index(), false);

    LOG_DBG("Finding relationships for type {}, {}, {}", cppast::to_string(t_),
        t_.kind(), fn);

    relationship_t relationship_type = relationship_hint;
    const auto &t = cppast::remove_cv(cx::util::unreferenced(t_));

    if (t.kind() == cppast::cpp_type_kind::array_t) {
        auto &a = static_cast<const cppast::cpp_array_type &>(t);
        found = find_relationships(
            a.value_type(), relationships, relationship_t::kDependency);
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
                cppast::to_string(t), relationship_t::kDependency);

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
                relationship_t::kDependency);
        }
        else if (name.find("std::shared_ptr") == 0) {
            found = find_relationships(args[0u].type().value(), relationships,
                relationship_t::kDependency);
        }
        else if (name.find("std::weak_ptr") == 0) {
            found = find_relationships(args[0u].type().value(), relationships,
                relationship_t::kDependency);
        }
        else if (name.find("std::vector") == 0) {
            found = find_relationships(args[0u].type().value(), relationships,
                relationship_t::kDependency);
        }
        else if (ctx.config().should_include(fn)) {
            LOG_DBG("User defined template instantiation: {} | {}",
                cppast::to_string(t_), cppast::to_string(t_.canonical()));

            relationships.emplace_back(
                cppast::to_string(t), relationship_t::kDependency);

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
