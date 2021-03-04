/**
 * src/uml/class_diagram_visitor.h
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
#pragma once

#include "class_diagram_model.h"
#include "cx/cursor.h"

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>

namespace clanguml {
namespace visitor {
namespace class_diagram {

using clanguml::model::class_diagram::class_;
using clanguml::model::class_diagram::class_member;
using clanguml::model::class_diagram::class_method;
using clanguml::model::class_diagram::class_parent;
using clanguml::model::class_diagram::class_relationship;
using clanguml::model::class_diagram::diagram;
using clanguml::model::class_diagram::enum_;
using clanguml::model::class_diagram::relationship_t;
using clanguml::model::class_diagram::scope_t;

struct tu_context {
    tu_context(diagram &d_, const clanguml::config::class_diagram &config_)
        : d{d_}
        , config{config_}
    {
    }

    std::vector<std::string> namespace_;
    class_diagram::diagram &d;
    const clanguml::config::class_diagram &config;
};

template <typename T> struct element_visitor_context {
    element_visitor_context(T &e)
        : element(e)
    {
    }
    tu_context *ctx;

    T &element;
};

enum CXChildVisitResult visit_if_cursor_valid(
    cx::cursor cursor, std::function<void(cx::cursor)> f)
{
    enum CXChildVisitResult ret = CXChildVisit_Break;
    if (cursor.is_definition() || cursor.is_declaration()) {
        if (!cursor.spelling().empty()) {
            f(cursor);
            ret = CXChildVisit_Continue;
        }
        else {
            ret = CXChildVisit_Recurse;
        }
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

void find_relationships(
    cx::type t, std::vector<std::pair<cx::type, relationship_t>> &relationships)
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
            find_relationships(template_arguments[0], relationships);
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

static enum CXChildVisitResult enum_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    auto ctx = (element_visitor_context<enum_> *)client_data;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    spdlog::info("Visiting enum {}: {} - {}:{}", ctx->element.name,
        cursor.spelling(), cursor.kind());

    enum CXChildVisitResult ret = CXChildVisit_Break;
    switch (cursor.kind()) {
        case CXCursor_EnumConstantDecl:
            visit_if_cursor_valid(cursor, [ctx](cx::cursor cursor) {
                spdlog::info("Adding enum constant {}::{}", ctx->element.name,
                    cursor.spelling());

                ctx->element.constants.emplace_back(cursor.spelling());
            });

            ret = CXChildVisit_Continue;
            break;
        default:
            ret = CXChildVisit_Continue;
            break;
    }

    return ret;
}

static enum CXChildVisitResult class_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    auto ctx = (element_visitor_context<class_> *)client_data;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    std::string cursor_name_str = cursor.spelling();

    spdlog::info("Visiting {}: {} - {}:{}",
        ctx->element.is_struct ? "struct" : "class", ctx->element.name,
        cursor_name_str, cursor.kind());

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
            if (!config.should_include(cursor.fully_qualified())) {
                ret = CXChildVisit_Continue;
                break;
            }

            visit_if_cursor_valid(cursor, [ctx, is_struct](cx::cursor cursor) {
                class_ c{};
                c.is_struct = is_struct;
                c.name = cursor.fully_qualified();
                c.namespace_ = ctx->ctx->namespace_;

                auto class_ctx = element_visitor_context<class_>(c);
                class_ctx.ctx = ctx->ctx;

                clang_visitChildren(cursor.get(), class_visitor, &class_ctx);

                class_relationship containment;
                containment.type = relationship_t::kContainment;
                containment.destination = c.name;
                ctx->element.relationships.emplace_back(std::move(containment));

                spdlog::info(
                    "Added relationship {} +-- {}", ctx->element.name, c.name);

                ctx->ctx->d.classes.emplace_back(std::move(c));
            });
            ret = CXChildVisit_Continue;
            break;
        case CXCursor_EnumDecl:
            if (!config.should_include(cursor.fully_qualified())) {
                ret = CXChildVisit_Continue;
                break;
            }

            visit_if_cursor_valid(cursor, [ctx, is_struct](cx::cursor cursor) {
                enum_ e{};
                e.name = cursor.fully_qualified();
                e.namespace_ = ctx->ctx->namespace_;

                auto enum_ctx = element_visitor_context<enum_>(e);
                enum_ctx.ctx = ctx->ctx;

                clang_visitChildren(cursor.get(), enum_visitor, &enum_ctx);

                class_relationship containment;
                containment.type = relationship_t::kContainment;
                containment.destination = e.name;
                ctx->element.relationships.emplace_back(std::move(containment));

                spdlog::info(
                    "Added relationship {} +-- {}", ctx->element.name, e.name);

                ctx->ctx->d.enums.emplace_back(std::move(e));
            });
            ret = CXChildVisit_Continue;
            break;
        case CXCursor_CXXMethod:
        case CXCursor_Constructor:
        case CXCursor_Destructor:
        case CXCursor_FunctionTemplate: {
            if (!config.should_include(cursor.fully_qualified())) {
                ret = CXChildVisit_Continue;
                break;
            }

            visit_if_cursor_valid(cursor, [ctx](cx::cursor cursor) {
                class_method m;
                m.name = cursor.spelling();
                m.type = cursor.type().result_type().spelling();
                m.is_pure_virtual = cursor.is_method_pure_virtual();
                m.is_virtual = cursor.is_method_virtual();
                m.is_const = cursor.is_method_const();
                m.is_defaulted = cursor.is_method_defaulted();
                m.is_static = cursor.is_method_static();
                m.scope =
                    cx_access_specifier_to_scope(cursor.cxxaccess_specifier());

                spdlog::info("Adding method {} {}::{}()", m.type,
                    ctx->element.name, cursor.spelling());

                ctx->element.methods.emplace_back(std::move(m));
            });
            ret = CXChildVisit_Continue;
            break;
        }
        case CXCursor_VarDecl:
            is_vardecl = true;
        case CXCursor_FieldDecl: {
            visit_if_cursor_valid(
                cursor, [ctx, &config, is_vardecl](cx::cursor cursor) {
                    auto t = cursor.type();
                    class_member m;
                    m.name = cursor.spelling();
                    m.type = t.is_template() ? t.unqualified()
                                             : t.canonical().unqualified();
                    m.scope = cx_access_specifier_to_scope(
                        cursor.cxxaccess_specifier());
                    m.is_static = cursor.is_static();

                    spdlog::info("Adding member {} {}::{} (type kind: {} | {} "
                                 "| {} | {} | {})",
                        m.type, ctx->element.name, cursor.spelling(),
                        t.kind_spelling(), t.pointee_type().spelling(),
                        t.is_pod(), t.canonical().spelling(),
                        t.is_relationship());

                    relationship_t relationship_type = relationship_t::kNone;

                    auto name = t.canonical().unqualified();
                    auto destination = name;

                    // Parse the field declaration to determine the relationship
                    // type
                    // Skip:
                    //  - POD
                    //  - function variables
                    spdlog::info(
                        "Analyzing possible relationship candidate: {}",
                        t.canonical().unqualified());

                    if (t.is_relationship() &&
                        (config.should_include(t.canonical().unqualified()) ||
                            t.is_template() || t.is_array())) {
                        std::vector<std::pair<cx::type, relationship_t>>
                            relationships{};
                        find_relationships(t, relationships);

                        for (const auto &[type, relationship_type] :
                            relationships) {
                            if (relationship_type != relationship_t::kNone) {
                                class_relationship r;
                                r.destination =
                                    type.referenced().canonical().unqualified();
                                r.type = relationship_type;
                                r.label = m.name;
                                ctx->element.relationships.emplace_back(
                                    std::move(r));

                                spdlog::info(
                                    "Added relationship to: {}", r.destination);
                            }
                        }
                    }

                    ctx->element.members.emplace_back(std::move(m));
                });
            ret = CXChildVisit_Continue;
            break;
        }
        case CXCursor_CXXBaseSpecifier: {
            if (!config.should_include(cursor.referenced().fully_qualified())) {
                ret = CXChildVisit_Continue;
                break;
            }

            auto ref_cursor = cursor.referenced();
            auto display_name = ref_cursor.referenced().spelling();

            auto base_access = cursor.cxxaccess_specifier();

            spdlog::error(
                "Found base specifier: {} - {}", cursor_name_str, display_name);

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

            ctx->element.bases.emplace_back(std::move(cp));

            ret = CXChildVisit_Continue;
            break;
        }
        default:
            ret = CXChildVisit_Continue;
            break;
    }

    return ret;
};

static enum CXChildVisitResult translation_unit_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    struct tu_context *ctx = (struct tu_context *)client_data;

    enum CXChildVisitResult ret = CXChildVisit_Break;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    if (clang_Location_isFromMainFile(cursor.location()) == 0) {
        return CXChildVisit_Continue;
    }
    std::string cursor_name_str = cursor.spelling();
    spdlog::debug("Visiting cursor: {}", cursor_name_str);

    bool is_struct{false};
    auto scope{scope_t::kPrivate};
    switch (cursor.kind()) {
        case CXCursor_StructDecl:
            spdlog::debug("Found struct declaration: {}", cursor_name_str);

            is_struct = true;

            [[fallthrough]];
        case CXCursor_ClassTemplate:
            [[fallthrough]];
        case CXCursor_ClassDecl: {
            spdlog::debug("Found class or class template declaration: {}",
                cursor_name_str);
            if (!ctx->config.should_include(cursor.fully_qualified())) {
                ret = CXChildVisit_Continue;
                break;
            }

            scope = scope_t::kPublic;

            visit_if_cursor_valid(cursor, [ctx, is_struct](cx::cursor cursor) {
                class_ c{};
                c.is_struct = is_struct;
                c.name = cursor.fully_qualified();
                c.namespace_ = ctx->namespace_;

                auto class_ctx = element_visitor_context<class_>(c);
                class_ctx.ctx = ctx;

                clang_visitChildren(cursor.get(), class_visitor, &class_ctx);

                ctx->d.classes.emplace_back(std::move(c));
            });

            ret = CXChildVisit_Continue;
            break;
        }
        case CXCursor_EnumDecl: {
            spdlog::debug("Found enum declaration: {}", cursor_name_str);
            if (!ctx->config.should_include(cursor.fully_qualified())) {
                ret = CXChildVisit_Continue;
                break;
            }

            visit_if_cursor_valid(cursor, [ctx, is_struct](cx::cursor cursor) {
                enum_ e{};
                e.name = cursor.fully_qualified();
                e.namespace_ = ctx->namespace_;

                auto enum_ctx = element_visitor_context<enum_>(e);
                enum_ctx.ctx = ctx;

                clang_visitChildren(cursor.get(), enum_visitor, &enum_ctx);

                ctx->d.enums.emplace_back(std::move(e));
            });
            ret = CXChildVisit_Continue;

            break;
        }
        case CXCursor_Namespace: {
            spdlog::debug("Found namespace specifier: {}", cursor_name_str);

            ret = CXChildVisit_Recurse;

            break;
        }
        default:
            spdlog::debug("Found cursor: {}", cursor_name_str);

            ret = CXChildVisit_Recurse;
    }

    return ret;
}
}
}
}
