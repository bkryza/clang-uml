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

template <typename T> struct element_visitor_context {
    element_visitor_context(T &e)
        : element(e)
    {
    }
    CXCursorKind current_cursor_kind;
    std::vector<std::string> namespace_;

    T &element;
};

struct class_visitor_context : element_visitor_context<class_> {
    class_visitor_context(class_ &c)
        : element_visitor_context<class_>(c)
    {
    }
    scope_t scope;
};

struct tu_context {
    tu_context(diagram &d_)
        : d{d_}
    {
    }

    std::vector<std::string> namespace_;
    class_diagram::diagram &d;
};

enum CXChildVisitResult visit_if_cursor_valid(
    cx::cursor cursor, std::function<void(cx::cursor)> f)
{
    enum CXChildVisitResult ret = CXChildVisit_Break;
    if (cursor.is_definition()) {
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

static enum CXChildVisitResult enum_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    auto e = (struct enum_ *)client_data;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    spdlog::info("Visiting enum {}: {} - {}:{}", e->name, cursor.spelling(),
        cursor.kind());

    enum CXChildVisitResult ret = CXChildVisit_Break;
    switch (cursor.kind()) {
        case CXCursor_EnumConstantDecl:
            visit_if_cursor_valid(cursor, [e](cx::cursor cursor) {
                spdlog::info(
                    "Adding enum constant {}::{}", e->name, cursor.spelling());

                e->constants.emplace_back(cursor.spelling());
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
    auto c = (struct class_ *)client_data;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    std::string cursor_name_str = cursor.spelling();

    spdlog::info("Visiting {}: {} - {}:{}", c->is_struct ? "struct" : "class",
        c->name, cursor_name_str, cursor.kind());

    enum CXChildVisitResult ret = CXChildVisit_Break;
    switch (cursor.kind()) {
        case CXCursor_CXXMethod:
        case CXCursor_Constructor:
        case CXCursor_Destructor:
        case CXCursor_FunctionTemplate: {
            visit_if_cursor_valid(cursor, [c](cx::cursor cursor) {
                class_method m;
                m.name = cursor.spelling();
                m.type = cursor.type().spelling();

                spdlog::info("Adding method {} {}::{}()", m.type, c->name,
                    cursor.spelling());

                c->methods.emplace_back(std::move(m));
            });
            ret = CXChildVisit_Continue;
            break;
        }
        case CXCursor_FieldDecl: {
            visit_if_cursor_valid(cursor, [c](cx::cursor cursor) {
                auto t = cursor.type();
                class_member m;
                m.name = cursor.spelling();
                m.type = cursor.type().spelling();

                spdlog::info("Adding member {} {}::{}", m.type, c->name,
                    cursor.spelling());

                relationship_t relationship_type = relationship_t::kOwnership;

                // Parse the field declaration to determine the relationship
                // type
                if (!t.is_pod()) {
                    while (true) {
                        if (t.kind() == CXType_Pointer) {
                            relationship_type = relationship_t::kAssociation;
                            t = t.pointee_type();
                            continue;
                        }
                        else if (t.kind() == CXType_LValueReference) {
                            relationship_type = relationship_t::kAggregation;
                            t = t.pointee_type();
                            continue;
                        }
                        else if (t.kind() == CXType_RValueReference) {
                            relationship_type = relationship_t::kAssociation;
                            t = t.pointee_type();
                            continue;
                        }
                        /*else if(t.kind == CXType_Elaborated) {
                            t = clang_Type_getNamedType(t);
                            continue;
                        }*/
                        else /*if (t.kind == CXType_Record) */ {
                            spdlog::error("UNKNOWN CXTYPE: {}", t.kind());
                            class_relationship r;
                            auto template_argument_count =
                                t.template_arguments_count();
                            std::string name = t.spelling();

                            if (template_argument_count > 0) {
                                std::vector<cx::type> template_arguments;
                                for (int i = 0; i < template_argument_count;
                                     i++) {
                                    auto tt = t.template_argument_type(i);
                                    template_arguments.push_back(tt);
                                }

                                if (name.rfind("vector") == 0 ||
                                    name.rfind("std::vector") == 0) {
                                    r.type = relationship_t::kAggregation;
                                    r.destination =
                                        template_arguments[0].spelling();
                                }
                                if (name.rfind("map") == 0 ||
                                    name.rfind("std::map") == 0) {
                                    r.type = relationship_t::kAggregation;
                                    r.destination =
                                        template_arguments[1].spelling();
                                }
                                r.label = m.name;
                                c->relationships.emplace_back(std::move(r));
                            }
                            else {
                                r.destination = name;
                                r.type = relationship_type;
                                r.label = m.name;
                                c->relationships.emplace_back(std::move(r));
                            }

                            spdlog::debug(
                                "Adding relationship to: {}", r.destination);
                        }
                        // else {
                        // spdlog::error("UNKNOWN CXTYPE: {}", t.kind);
                        //}
                        break;
                    }
                }

                c->members.emplace_back(std::move(m));
            });
            ret = CXChildVisit_Continue;
            break;
        }
        case CXCursor_CXXBaseSpecifier: {
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

            c->bases.emplace_back(std::move(cp));

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

            scope = scope_t::kPublic;

            visit_if_cursor_valid(cursor, [ctx, is_struct](cx::cursor cursor) {
                class_ c{};
                c.is_struct = is_struct;
                c.name = cursor.spelling();
                c.namespace_ = ctx->namespace_;

                clang_visitChildren(cursor.get(), class_visitor, &c);

                ctx->d.classes.emplace_back(std::move(c));
            });

            ret = CXChildVisit_Continue;
            break;
        }
        case CXCursor_EnumDecl: {
            spdlog::debug("Found enum declaration: {}", cursor_name_str);

            visit_if_cursor_valid(cursor, [ctx, is_struct](cx::cursor cursor) {
                enum_ e{};
                e.name = cursor.spelling();
                e.namespace_ = ctx->namespace_;

                clang_visitChildren(cursor.get(), enum_visitor, &e);

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
