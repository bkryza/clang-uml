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

#include <cppast/cpp_array_type.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <cppast/cpp_template.hpp>
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
    std::string prefix;
    // visit each entity in the file
    cppast::visit(file,
        [&, this](const cppast::cpp_entity &e, cppast::visitor_info info) {
            if (e.kind() == cppast::cpp_entity_kind::class_t) {
                spdlog::debug("{}'{}' - {}", prefix, cx::util::full_name(e),
                    cppast::to_string(e.kind()));

                auto &cls = static_cast<const cppast::cpp_class &>(e);

                if (ctx.config.should_include(cx::util::fully_prefixed(cls)))
                    process_class_declaration(cls);
            }
            else if (e.kind() == cppast::cpp_entity_kind::enum_t) {
                spdlog::debug("{}'{}' - {}", prefix, cx::util::full_name(e),
                    cppast::to_string(e.kind()));

                auto &enm = static_cast<const cppast::cpp_enum &>(e);

                if (ctx.config.should_include(cx::util::fully_prefixed(enm)))
                    process_enum_declaration(enm);
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

            spdlog::debug("Added relationship {} +-- {}", e.name,
                containment.destination);
            break;
        }
    }

    ctx.d.add_enum(std::move(e));
}

void tu_visitor::process_class_declaration(const cppast::cpp_class &cls)
{
    class_ c;
    c.usr = cx::util::full_name(cls);
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
        else if (child.kind() == cppast::cpp_entity_kind::constructor_t) {
            auto &mc = static_cast<const cppast::cpp_constructor &>(child);
            process_constructor(mc, c, last_access_specifier);
        }
        else if (child.kind() == cppast::cpp_entity_kind::destructor_t) {
            auto &mc = static_cast<const cppast::cpp_destructor &>(child);
            process_destructor(mc, c, last_access_specifier);
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
        auto scope = cppast::cpp_scope_name(type_safe::ref(cls));
        for (const auto &tp : scope.template_parameters()) {
            if (tp.kind() ==
                cppast::cpp_entity_kind::template_type_parameter_t) {
                process_template_type_parameter(
                    static_cast<const cppast::cpp_template_type_parameter &>(
                        tp),
                    c);
            }
            else if (tp.kind() ==
                cppast::cpp_entity_kind::non_type_template_parameter_t) {
                process_template_nontype_parameter(
                    static_cast<
                        const cppast::cpp_non_type_template_parameter &>(tp),
                    c);
            }
            else if (tp.kind() ==
                cppast::cpp_entity_kind::template_template_parameter_t) {
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
            c.relationships.emplace_back(std::move(containment));

            spdlog::debug("Added relationship {} +-- {}",
                c.full_name(ctx.config.using_namespace),
                containment.destination);
            break;
        }
    }

    ctx.d.add_class(std::move(c));
}

void tu_visitor::process_field(const cppast::cpp_member_variable &mv, class_ &c,
    cppast::cpp_access_specifier_kind as)
{
    class_member m;
    m.name = mv.name();
    m.type = cppast::to_string(mv.type());
    m.scope = detail::cpp_access_specifier_to_scope(as);
    m.is_static = false;

    if (mv.type().kind() != cppast::cpp_type_kind::builtin_t) {
        std::vector<std::pair<std::string, relationship_t>> relationships;

        find_relationships(mv.type(), relationships);

        for (const auto &[type, relationship_type] : relationships) {
            if (relationship_type != relationship_t::kNone) {
                class_relationship r;
                r.destination = type;
                r.type = relationship_type;
                r.label = m.name;
                c.relationships.emplace_back(std::move(r));

                spdlog::debug("Added relationship to: {}", r.destination);
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
    m.is_defaulted = false; // cursor.is_method_defaulted();
    m.is_static = false;    // cppast::is_static(mf.storage_class());
    m.scope = detail::cpp_access_specifier_to_scope(as);

    for (auto &param : mf.parameters())
        process_function_parameter(param, m);

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
        process_function_parameter(param, m);

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
        process_function_parameter(param, m);

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
    const cppast::cpp_function_parameter &param, class_method &m)
{
    method_parameter mp;
    mp.name = param.name();
    mp.type = cppast::to_string(param.type());

    auto dv = param.default_value();
    if (dv)
        switch (dv.value().kind()) {
            case cppast::cpp_expression_kind::literal_t:
                mp.default_value =
                    static_cast<const cppast::cpp_literal_expression &>(
                        dv.value())
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

void tu_visitor::find_relationships(const cppast::cpp_type &t_,
    std::vector<std::pair<std::string, relationship_t>> &relationships,
    relationship_t relationship_hint)
{
    relationship_t relationship_type = relationship_hint;
    const auto &t = cppast::remove_cv(t_);

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
            spdlog::debug(
                "Template instantiation {} has no exposed arguments", name);

            return;
        }

        const auto &args = tinst.arguments().value();

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
                relationship_t::kAggregation);
        }
        else {
            for (const auto &arg : args) {
                find_relationships(
                    arg.type().value(), relationships, relationship_type);
            }
        }
    }
    else if (t.kind() == cppast::cpp_type_kind::user_defined_t) {
        if (relationship_type != relationship_t::kNone)
            relationships.emplace_back(cppast::to_string(t), relationship_hint);
        else
            relationships.emplace_back(
                cppast::to_string(t), relationship_t::kComposition);
    }
    else if (t.kind() == cppast::cpp_type_kind::pointer_t) {
        auto &p = static_cast<const cppast::cpp_pointer_type &>(t);
        find_relationships(
            p.pointee(), relationships, relationship_t::kAssociation);
    }
    else if (t.kind() == cppast::cpp_type_kind::reference_t) {
        auto &r = static_cast<const cppast::cpp_reference_type &>(t);
        auto rt = relationship_t::kAssociation;
        if (r.reference_kind() == cppast::cpp_reference::cpp_ref_rvalue) {
            rt = relationship_t::kComposition;
        }
        find_relationships(r.referee(), relationships, rt);
    }
}

//
// ============== OLD CODE =======================
//

enum CXChildVisitResult visit_if_cursor_valid(
    cx::cursor cursor, std::function<enum CXChildVisitResult(cx::cursor)> f)
{
    enum CXChildVisitResult ret = CXChildVisit_Break;

    if (cursor.is_definition() || cursor.is_declaration()) {
        if (!cursor.spelling().empty())
            ret = f(cursor);
        else
            ret = CXChildVisit_Recurse;
    }
    else {
        ret = CXChildVisit_Continue;
    }

    return ret;
}

scope_t cx_access_specifier_to_scope(CX_CXXAccessSpecifier as)
{
    scope_t res = scope_t::kPublic;
    switch (as) {
        case CX_CXXAccessSpecifier::CX_CXXPublic:
            res = scope_t::kPublic;
            break;
        case CX_CXXAccessSpecifier::CX_CXXPrivate:
            res = scope_t::kPrivate;
            break;
        case CX_CXXAccessSpecifier::CX_CXXProtected:
            res = scope_t::kProtected;
            break;
        default:
            break;
    }

    return res;
}

void find_relationships(cx::type t,
    std::vector<std::pair<cx::type, relationship_t>> &relationships,
    relationship_t relationship_hint)
{
    relationship_t relationship_type = relationship_t::kNone;

    if (t.is_array()) {
        find_relationships(t.array_type(), relationships);
        return;
    }

    auto name = t.canonical().unqualified();

    const auto template_argument_count = t.template_arguments_count();

    if (template_argument_count > 0) {
        class_relationship r;

        std::vector<cx::type> template_arguments;
        for (int i = 0; i < template_argument_count; i++) {
            auto tt = t.template_argument_type(i);
            template_arguments.push_back(tt);
        }

        if (name.find("std::unique_ptr") == 0) {
            find_relationships(template_arguments[0], relationships);
        }
        else if (name.find("std::shared_ptr") == 0) {
            find_relationships(template_arguments[0], relationships,
                relationship_t::kAssociation);
        }
        else if (name.find("std::weak_ptr") == 0) {
            find_relationships(template_arguments[0], relationships,
                relationship_t::kAssociation);
        }
        else if (name.find("std::vector") == 0) {
            find_relationships(template_arguments[0], relationships);
        }
        else if (name.find("std::array") == 0) {
            find_relationships(template_arguments[0], relationships);
        }
        else {
            for (const auto type : template_arguments) {
                find_relationships(type, relationships);
            }
        }
    }
    else if (t.kind() == CXType_Record) {
        if (relationship_hint != relationship_t::kNone)
            relationships.emplace_back(t, relationship_hint);
        else
            relationships.emplace_back(t, relationship_t::kOwnership);
    }
    else if (t.kind() == CXType_Pointer) {
        relationships.emplace_back(t, relationship_t::kAssociation);
    }
    else if (t.kind() == CXType_LValueReference) {
        relationships.emplace_back(t, relationship_t::kAssociation);
    }
    else if (t.kind() == CXType_RValueReference) {
        relationships.emplace_back(t, relationship_t::kOwnership);
    }
}

enum CXChildVisitResult enum_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    auto ctx = (element_visitor_context<enum_> *)client_data;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    spdlog::debug("Visiting enum {}: {} - {}:{}", ctx->element.name,
        cursor.spelling(), cursor.kind());

    enum CXChildVisitResult ret = CXChildVisit_Break;
    switch (cursor.kind()) {
        case CXCursor_EnumConstantDecl:
            ret = visit_if_cursor_valid(cursor, [ctx](cx::cursor cursor) {
                spdlog::debug("Adding enum constant {}::{}", ctx->element.name,
                    cursor.spelling());

                ctx->element.constants.emplace_back(cursor.spelling());
                return CXChildVisit_Continue;
            });
            break;
        default:
            ret = CXChildVisit_Continue;
            break;
    }

    return ret;
}

enum CXChildVisitResult method_parameter_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    auto ctx = (element_visitor_context<class_method> *)client_data;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    spdlog::debug("Visiting method declaration {}: {} - {}:{}",
        ctx->element.name, cursor.spelling(), cursor.kind(),
        cursor.referenced());

    enum CXChildVisitResult ret = CXChildVisit_Break;
    switch (cursor.kind()) {
        case CXCursor_ParmDecl: {
            spdlog::debug("Analyzing method parameter: {}, {}, {}", cursor,
                cursor.type().referenced(),
                cursor.type().referenced().type_declaration());

            auto t = cursor.type();
            method_parameter mp;
            mp.name = cursor.spelling();
            mp.type = t.spelling();
            mp.default_value = cursor.default_value();

            ctx->element.parameters.emplace_back(std::move(mp));
            std::string rdestination{};

            if (t.is_relationship()) {
                if (t.is_template_instantiation()) {
                    rdestination = t.referenced().instantiation_template();
                }
                else if (t.spelling().find('<') != std::string::npos) {
                    rdestination =
                        t.referenced().type_declaration().fully_qualified();
                }
                else {
                    rdestination = t.referenced().spelling();
                }

                if (ctx->ctx->config.should_include(rdestination) &&
                    rdestination != ctx->parent_class->name) {

                    spdlog::debug("Adding dependency to {} \n\tCURSOR={} "
                                  "\n\tREFTYPE={} \n\tTYPEDECL={}",
                        t.referenced().spelling(), cursor, t.referenced(),
                        t.referenced().type_declaration());

                    class_relationship r;
                    r.type = relationship_t::kDependency;

                    if (t.referenced().is_template_instantiation() &&
                        (t.referenced().type_declaration().kind() !=
                                CXCursor_InvalidFile ||
                            t.referenced()
                                    .type_declaration()
                                    .specialized_cursor_template()
                                    .kind() != CXCursor_InvalidFile)) {
                        class_ tinst = build_template_instantiation(
                            cursor, t.referenced());

                        // Add template instantiation relationship
                        class_relationship ri;
                        ri.destination = tinst.base_template_usr;
                        ri.type = relationship_t::kInstantiation;
                        ri.label = "";
                        tinst.add_relationship(std::move(ri));

                        r.destination = tinst.usr;

                        ctx->d.add_class(std::move(tinst));
                    }
                    else
                        r.destination = t.referenced().type_declaration().usr();

                    assert(ctx->parent_class != nullptr);

                    if ((r.destination != ctx->parent_class->name) &&
                        (r.destination != ctx->parent_class->usr))
                        ctx->parent_class->add_relationship(std::move(r));
                }

                ret = CXChildVisit_Continue;
            }
        } break;
        default:
            ret = CXChildVisit_Continue;
            break;
    }

    return ret;
}

enum CXChildVisitResult friend_class_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    auto ctx = (element_visitor_context<class_> *)client_data;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    spdlog::debug("Visiting friend class declaration {}: {} - {}:{}",
        ctx->element.name, cursor.spelling(), cursor.kind(),
        cursor.referenced());

    enum CXChildVisitResult ret = CXChildVisit_Break;
    switch (cursor.kind()) {
        case CXCursor_TemplateRef:
        case CXCursor_ClassTemplate:
        case CXCursor_TypeRef: {
            spdlog::debug("Analyzing friend declaration: {}, {}", cursor,
                cursor.specialized_cursor_template());

            if (!ctx->ctx->config.should_include(
                    cursor.referenced().fully_qualified())) {
                ret = CXChildVisit_Continue;
                break;
            }

            class_relationship r;
            r.type = relationship_t::kFriendship;
            r.label = "<<friend>>";
            r.destination = cursor.referenced().usr();

            ctx->element.relationships.emplace_back(std::move(r));

            ret = CXChildVisit_Continue;
        } break;
        default:
            ret = CXChildVisit_Continue;
            break;
    }

    return ret;
}

enum CXChildVisitResult process_class_base_specifier(
    cx::cursor cursor, class_ *parent, struct tu_context *ctx)
{
    if (!ctx->config.should_include(cursor.referenced().fully_qualified()))
        return CXChildVisit_Continue;

    auto ref_cursor = cursor.referenced();
    auto display_name = ref_cursor.referenced().spelling();

    auto base_access = cursor.cxxaccess_specifier();

    spdlog::debug(
        "Found base specifier: {} - {}", cursor.spelling(), display_name);

    class_parent cp;
    cp.name = display_name;
    cp.is_virtual = false;
    switch (base_access) {
        case CX_CXXAccessSpecifier::CX_CXXPrivate:
            cp.access = class_parent::access_t::kPrivate;
            break;
        case CX_CXXAccessSpecifier::CX_CXXPublic:
            cp.access = class_parent::access_t::kPublic;
            break;
        case CX_CXXAccessSpecifier::CX_CXXProtected:
            cp.access = class_parent::access_t::kProtected;
            break;
        default:
            cp.access = class_parent::access_t::kPublic;
    }

    parent->bases.emplace_back(std::move(cp));

    return CXChildVisit_Continue;
}

enum CXChildVisitResult process_template_type_parameter(
    cx::cursor cursor, class_ *parent, struct tu_context *ctx)
{
    spdlog::debug("Found template type parameter: {}: {}, isvariadic={}",
        cursor, cursor.type(), cursor.is_template_parameter_variadic());

    class_template ct;
    ct.type = "";
    ct.default_value = "";
    ct.is_variadic = cursor.is_template_parameter_variadic();
    ct.name = cursor.spelling();
    if (ct.is_variadic)
        ct.name += "...";
    parent->templates.emplace_back(std::move(ct));

    return CXChildVisit_Continue;
}

enum CXChildVisitResult process_template_nontype_parameter(
    cx::cursor cursor, class_ *parent, struct tu_context *ctx)
{
    spdlog::debug("Found template nontype parameter: {}: {}, isvariadic={}",
        cursor.spelling(), cursor.type(),
        cursor.is_template_parameter_variadic());

    class_template ct;
    ct.type = cursor.type().canonical().spelling();
    ct.name = cursor.spelling();
    ct.default_value = "";
    ct.is_variadic = cursor.is_template_parameter_variadic();
    if (ct.is_variadic)
        ct.name += "...";
    parent->templates.emplace_back(std::move(ct));

    return CXChildVisit_Continue;
}

enum CXChildVisitResult process_template_template_parameter(
    cx::cursor cursor, class_ *parent, struct tu_context *ctx)
{
    spdlog::debug("Found template template parameter: {}: {}",
        cursor.spelling(), cursor.type());

    class_template ct;
    ct.type = "";
    ct.name = cursor.spelling() + "<>";
    ct.default_value = "";
    parent->templates.emplace_back(std::move(ct));

    return CXChildVisit_Continue;
}

enum CXChildVisitResult process_method(
    cx::cursor cursor, class_ *parent, struct tu_context *ctx)
{
    if (!ctx->config.should_include(cursor.fully_qualified()))
        return CXChildVisit_Continue;

    class_method m;
    m.name = cursor.spelling();
    m.type = cursor.type().result_type().spelling();
    m.is_pure_virtual = cursor.is_method_pure_virtual();
    m.is_virtual = cursor.is_method_virtual();
    m.is_const = cursor.is_method_const();
    m.is_defaulted = cursor.is_method_defaulted();
    m.is_static = cursor.is_method_static();
    m.scope = cx_access_specifier_to_scope(cursor.cxxaccess_specifier());

    auto method_ctx = element_visitor_context<class_method>(ctx->d, m);
    method_ctx.ctx = ctx;
    method_ctx.parent_class = parent;

    clang_visitChildren(cursor.get(), method_parameter_visitor, &method_ctx);

    spdlog::debug("Adding method {} {}::{}()", cursor.type().result_type(),
        parent->name, cursor.spelling());

    parent->methods.emplace_back(std::move(m));

    return CXChildVisit_Continue;
}

enum CXChildVisitResult process_class_declaration(
    cx::cursor cursor, bool is_struct, class_ *parent, struct tu_context *ctx)
{
    if (!ctx->config.should_include(cursor.fully_qualified()))
        return CXChildVisit_Continue;

    if (cursor.is_forward_declaration())
        return CXChildVisit_Continue;

    class_ c;
    c.usr = cursor.usr();
    c.is_struct = is_struct;
    c.name = cursor.fully_qualified();
    c.namespace_ = ctx->namespace_;

    spdlog::debug("Class {} has {} template arguments.", c.name,
        cursor.template_argument_count());

    auto class_ctx = element_visitor_context<class_>(ctx->d, c);
    class_ctx.ctx = ctx;

    clang_visitChildren(cursor.get(), class_visitor, &class_ctx);

    if (parent != nullptr) {
        class_relationship containment;
        containment.type = relationship_t::kContainment;
        containment.destination = c.name;
        parent->relationships.emplace_back(std::move(containment));

        spdlog::debug("Added relationship {} +-- {}", parent->name, c.name);
    }

    ctx->d.add_class(std::move(c));

    return CXChildVisit_Continue;
}

enum CXChildVisitResult process_enum_declaration(
    cx::cursor cursor, class_ *parent, struct tu_context *ctx)
{
    if (!ctx->config.should_include(cursor.fully_qualified()))
        return CXChildVisit_Continue;

    enum_ e{};
    e.name = cursor.fully_qualified();
    e.namespace_ = ctx->namespace_;

    auto enum_ctx = element_visitor_context<enum_>(ctx->d, e);
    enum_ctx.ctx = ctx;

    clang_visitChildren(cursor.get(), enum_visitor, &enum_ctx);

    if (parent != nullptr) {
        class_relationship containment;
        containment.type = relationship_t::kContainment;
        containment.destination = e.name;
        parent->relationships.emplace_back(std::move(containment));

        spdlog::debug("Added relationship {} +-- {}", parent->name, e.name);
    }

    ctx->d.add_enum(std::move(e));

    return CXChildVisit_Continue;
}

class_ build_template_instantiation(cx::cursor cursor, cx::type t)
{
    auto template_type = t.type_declaration().specialized_cursor_template();

    spdlog::debug("Found template instantiation: {} ..|> {}", t, template_type);

    bool partial_specialization = false;
    if (template_type.kind() == CXCursor_InvalidFile) {
        partial_specialization = true;
        template_type = t.type_declaration();
    }

    class_ tinst;
    tinst.name = template_type.fully_qualified();
    tinst.is_template_instantiation = true;
    if (partial_specialization) {
        tinst.usr = template_type.usr();
    }
    else {
        tinst.usr = t.type_declaration().usr();
    }

    const auto &instantiation_params = cursor.tokenize_template_parameters();

    for (const auto &template_param : instantiation_params) {

        class_template ct;
        ct.type = template_param;
        tinst.templates.emplace_back(std::move(ct));

        spdlog::debug("Adding template argument '{}'", template_param);
    }

    tinst.base_template_usr = template_type.usr();

    return tinst;
}

bool process_template_specialization_class_field(
    cx::cursor cursor, cx::type t, class_ *parent, struct tu_context *ctx)
{
    auto tr = t.referenced();
    if (tr.spelling().find('<') != std::string::npos &&
        (tr.type_declaration().kind() != CXCursor_InvalidFile ||
            tr.type_declaration().specialized_cursor_template().kind() !=
                CXCursor_InvalidFile)) {

        class_ tinst = build_template_instantiation(cursor, tr);

        class_relationship r;
        r.destination = tinst.base_template_usr;
        r.type = relationship_t::kInstantiation;
        r.label = "";

        class_relationship a;

        bool partial_specialization = false;
        auto template_type =
            tr.type_declaration().specialized_cursor_template();

        // Check if this is partial specialization
        // TODO: Is there a better way to do it?
        if (template_type.kind() == CXCursor_InvalidFile) {
            partial_specialization = true;
            template_type = tr.type_declaration();
        }

        if (partial_specialization) {
            a.destination = tr.spelling();
        }
        else {
            a.destination = tinst.usr;
        }
        if (t.is_pointer() || t.is_reference())
            a.type = relationship_t::kAssociation;
        else
            a.type = relationship_t::kComposition;

        a.label = cursor.spelling();

        parent->relationships.emplace_back(std::move(a));

        tinst.add_relationship(std::move(r));

        ctx->d.add_class(std::move(tinst));
        return true;
    }

    return false;
}

enum CXChildVisitResult process_field(
    cx::cursor cursor, class_ *parent, struct tu_context *ctx)
{
    bool added_relation_to_instantiation{false};
    auto t = cursor.type();
    auto tr = t.referenced();

    class_member m;
    m.name = cursor.spelling();

    if (tr.is_template())
        m.type = t.unqualified();
    else if (tr.is_template_parameter())
        m.type = t.spelling();
    else
        m.type = t.canonical().unqualified();

    m.scope = cx_access_specifier_to_scope(cursor.cxxaccess_specifier());

    m.is_static = cursor.is_static();

    spdlog::debug(
        "Adding member {} {}::{} "
        "\n\tCURSOR={}\n\tTYPE={}\n\tTYPEDECL={}\n\tUNDERLYINGTYPE={}",
        m.type, parent->name, cursor.spelling(), cursor, t,
        tr.type_declaration(), tr.type_declaration().underlying_type());

    if (tr.is_unexposed()) {
        added_relation_to_instantiation =
            process_template_specialization_class_field(cursor, t, parent, ctx);
    }
    if (!added_relation_to_instantiation) {
        relationship_t relationship_type = relationship_t::kNone;

        auto name = t.canonical().unqualified();
        auto destination = name;

        // Parse the field declaration to determine the
        // relationship type Skip:
        //  - POD
        //  - function variables
        spdlog::debug("Analyzing possible relationship candidate: {}",
            t.canonical().unqualified());

        if (t.is_relationship() &&
            (ctx->config.should_include(t.canonical().unqualified()) ||
                t.is_template() || t.is_array())) {
            std::vector<std::pair<cx::type, relationship_t>> relationships{};
            find_relationships(t, relationships);

            for (const auto &[type, relationship_type] : relationships) {
                if (relationship_type != relationship_t::kNone) {
                    class_relationship r;
                    r.destination = type.referenced().canonical().unqualified();
                    r.type = relationship_type;
                    r.label = m.name;
                    parent->relationships.emplace_back(std::move(r));

                    spdlog::debug("Added relationship to: {}", r.destination);
                }
            }
        }
    }

    parent->members.emplace_back(std::move(m));

    return CXChildVisit_Continue;
}

enum CXChildVisitResult class_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    auto ctx = (element_visitor_context<class_> *)client_data;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    std::string cursor_name_str = cursor.spelling();

    spdlog::debug("Visiting {}: {} - {}",
        ctx->element.is_struct ? "struct" : "class", ctx->element.name, cursor);

    auto &config = ctx->ctx->config;

    enum CXChildVisitResult ret = CXChildVisit_Break;
    bool is_constructor{false};
    bool is_destructor{false};
    bool is_struct{false};
    bool is_vardecl{false};
    switch (cursor.kind()) {
        case CXCursor_StructDecl:
            is_struct = true;
        case CXCursor_ClassDecl:
        case CXCursor_ClassTemplate:
            ret = visit_if_cursor_valid(
                cursor, [ctx, is_struct](cx::cursor cursor) {
                    return process_class_declaration(
                        cursor, is_struct, &ctx->element, ctx->ctx);
                });
            break;
        case CXCursor_EnumDecl:
            ret = visit_if_cursor_valid(cursor, [ctx](cx::cursor cursor) {
                return process_enum_declaration(
                    cursor, &ctx->element, ctx->ctx);
            });
            break;
        case CXCursor_TemplateTypeParameter:
            ret = process_template_type_parameter(
                cursor, &ctx->element, ctx->ctx);
            break;
        case CXCursor_NonTypeTemplateParameter:
            ret = process_template_nontype_parameter(
                cursor, &ctx->element, ctx->ctx);
            break;
        case CXCursor_TemplateTemplateParameter:
            ret = process_template_template_parameter(
                cursor, &ctx->element, ctx->ctx);
            break;
        case CXCursor_CXXMethod:
        case CXCursor_Constructor:
        case CXCursor_Destructor:
        case CXCursor_FunctionTemplate: {
            ret = visit_if_cursor_valid(cursor, [ctx](cx::cursor cursor) {
                return process_method(cursor, &ctx->element, ctx->ctx);
            });
            break;
        }
        case CXCursor_VarDecl:
            is_vardecl = true;
        case CXCursor_FieldDecl: {
            ret = visit_if_cursor_valid(
                cursor, [ctx, &config, is_vardecl](cx::cursor cursor) {
                    return process_field(cursor, &ctx->element, ctx->ctx);
                });
            break;
        }
        case CXCursor_ClassTemplatePartialSpecialization: {
            spdlog::debug("Found template specialization: {}", cursor);
            ret = CXChildVisit_Continue;
        } break;
        case CXCursor_CXXBaseSpecifier:
            ret = process_class_base_specifier(cursor, &ctx->element, ctx->ctx);
            break;
        case CXCursor_FriendDecl: {
            clang_visitChildren(cursor.get(), friend_class_visitor, ctx);

            ret = CXChildVisit_Continue;
        } break;
        default:
            ret = CXChildVisit_Continue;
            break;
    }

    return ret;
};

enum CXChildVisitResult translation_unit_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    struct tu_context *ctx = (struct tu_context *)client_data;

    enum CXChildVisitResult ret = CXChildVisit_Break;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    if (clang_Location_isFromMainFile(cursor.location()) == 0)
        return CXChildVisit_Continue;

    spdlog::debug("Visiting cursor: {}", cursor.spelling());

    bool is_struct{false};
    auto scope{scope_t::kPrivate};
    switch (cursor.kind()) {
        case CXCursor_StructDecl:
            spdlog::debug("Found struct declaration: {}", cursor.spelling());
            is_struct = true;

            [[fallthrough]];
        case CXCursor_ClassTemplate:
            [[fallthrough]];
        case CXCursor_ClassDecl: {
            spdlog::debug(
                "Found class or class template declaration: {}", cursor);
            scope = scope_t::kPublic;
            ret = visit_if_cursor_valid(
                cursor, [ctx, is_struct](cx::cursor cursor) {
                    return process_class_declaration(
                        cursor, is_struct, nullptr, ctx);
                });
            break;
        }
        case CXCursor_EnumDecl: {
            spdlog::debug("Found enum declaration: {}", cursor.spelling());
            ret = visit_if_cursor_valid(
                cursor, [ctx, is_struct](cx::cursor cursor) {
                    return process_enum_declaration(cursor, nullptr, ctx);
                });
            break;
        }
        case CXCursor_Namespace: {
            spdlog::debug("Found namespace specifier: {}", cursor.spelling());
            ret = CXChildVisit_Recurse;
            break;
        }
        default:
            spdlog::debug("Found cursor: {}", cursor.spelling());
            ret = CXChildVisit_Recurse;
            break;
    }

    return ret;
}
}
}
}
