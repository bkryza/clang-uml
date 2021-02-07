#pragma once

#include "class_diagram_model.h"

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
    tu_context(diagram &d)
        : diagram(d)
    {
    }

    std::vector<std::string> namespace_;
    diagram &diagram;
};

enum CXChildVisitResult visit_if_cursor_valid(
    CXCursor cursor, std::function<void(CXCursor)> f)
{
    enum CXChildVisitResult ret = CXChildVisit_Break;
    CXString cursorName = clang_getCursorSpelling(cursor);
    if (clang_isCursorDefinition(cursor)) {
        if (*clang_getCString(cursorName)) {
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
    CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    auto e = (struct enum_ *)client_data;

    CXString cursorName = clang_getCursorSpelling(cursor);
    std::string cursor_name_str = clang_getCString(cursorName);

    spdlog::info(
        "Visiting enum {}: {} - {}:{}", e->name, cursor_name_str, cursor.kind);

    enum CXChildVisitResult ret = CXChildVisit_Break;
    switch (cursor.kind) {
        case CXCursor_EnumConstantDecl:
            visit_if_cursor_valid(
                cursor, [e, cursor_name_str](CXCursor cursor) {
                    spdlog::info("Adding enum constant {}::{}", e->name,
                        cursor_name_str);

                    e->constants.emplace_back(cursor_name_str);
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
    CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    auto c = (struct class_ *)client_data;

    CXString cursorName = clang_getCursorSpelling(cursor);
    std::string cursor_name_str = clang_getCString(cursorName);

    spdlog::info("Visiting {}: {} - {}:{}", c->is_struct ? "struct" : "class",
        c->name, cursor_name_str, cursor.kind);

    enum CXChildVisitResult ret = CXChildVisit_Break;
    switch (cursor.kind) {
        case CXCursor_CXXMethod:
        case CXCursor_Constructor:
        case CXCursor_Destructor:
        case CXCursor_FunctionTemplate: {
            visit_if_cursor_valid(
                cursor, [c, cursor_name_str](CXCursor cursor) {
                    class_method m;
                    m.name = cursor_name_str;
                    m.type = clang_getCString(clang_getTypeSpelling(
                        clang_getResultType(clang_getCursorType(cursor))));

                    spdlog::info("Adding method {} {}::{}()", m.type, c->name,
                        cursor_name_str);

                    c->methods.emplace_back(std::move(m));
                });
            ret = CXChildVisit_Continue;
            break;
        }
        case CXCursor_FieldDecl: {
            visit_if_cursor_valid(cursor, [c, cursor_name_str](CXCursor cursor) {
                auto t = clang_getCursorType(cursor);
                class_member m;
                m.name = cursor_name_str;
                m.type = clang_getCString(clang_getTypeSpelling(t));

                spdlog::info("Adding member {} {}::{}", m.type, c->name,
                    cursor_name_str);

                relationship_t relationship_type = relationship_t::kOwnership;

                // Parse the field declaration to determine the relationship
                // type
                if (!clang_isPODType(t)) {
                    while (true) {
                        if (t.kind == CXType_Pointer) {
                            relationship_type = relationship_t::kAssociation;
                            t = clang_getPointeeType(t);
                            continue;
                        }
                        else if (t.kind == CXType_LValueReference) {
                            relationship_type = relationship_t::kAggregation;
                            t = clang_getPointeeType(t);
                            continue;
                        }
                        else if (t.kind == CXType_RValueReference) {
                            relationship_type = relationship_t::kAssociation;
                            t = clang_getPointeeType(t);
                            continue;
                        }
                        /*else if(t.kind == CXType_Elaborated) {
                            t = clang_Type_getNamedType(t);
                            continue;
                        }*/
                        else /*if (t.kind == CXType_Record) */ {
                            spdlog::error("UNKNOWN CXTYPE: {}", t.kind);
                            class_relationship r;
                            auto template_argument_count =
                                clang_Type_getNumTemplateArguments(t);
                            std::string name{
                                clang_getCString(clang_getTypeSpelling(t))};

                            if (template_argument_count > 0) {
                                std::vector<CXType> template_arguments;
                                for (int i = 0; i < template_argument_count;
                                     i++) {
                                    auto tt =
                                        clang_Type_getTemplateArgumentAsType(
                                            t, i);
                                    template_arguments.push_back(tt);
                                }

                                if (name.rfind("vector") == 0 ||
                                    name.rfind("std::vector") == 0) {
                                    r.type = relationship_t::kAggregation;
                                    r.destination =
                                        clang_getCString(clang_getTypeSpelling(
                                            template_arguments[0]));
                                }
                                if (name.rfind("map") == 0 ||
                                    name.rfind("std::map") == 0) {
                                    r.type = relationship_t::kAggregation;
                                    r.destination =
                                        clang_getCString(clang_getTypeSpelling(
                                            template_arguments[1]));
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
            CXCursor ref_cursor = clang_getCursorReferenced(cursor);
            CXString display_name = clang_getCursorDisplayName(ref_cursor);

            auto base_access = clang_getCXXAccessSpecifier(cursor);

            spdlog::error("Found base specifier: {} - {}", cursor_name_str,
                clang_getCString(display_name));

            class_parent cp;
            cp.name = clang_getCString(display_name);
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
    CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0) {
        return CXChildVisit_Continue;
    }

    struct tu_context *ctx = (struct tu_context *)client_data;

    enum CXChildVisitResult ret = CXChildVisit_Break;

    CXString cursorName = clang_getCursorSpelling(cursor);
    std::string cursor_name_str = clang_getCString(cursorName);

    spdlog::debug("Visiting cursor: {}", cursor_name_str);

    bool is_struct{false};
    auto scope{scope_t::kPrivate};
    switch (cursor.kind) {
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

            visit_if_cursor_valid(
                cursor, [ctx, is_struct, cursor_name_str](CXCursor cursor) {
                    class_ c{};
                    c.is_struct = is_struct;
                    c.name = cursor_name_str;
                    c.namespace_ = ctx->namespace_;

                    clang_visitChildren(cursor, class_visitor, &c);

                    ctx->diagram.classes.emplace_back(std::move(c));
                });

            ret = CXChildVisit_Continue;
            break;
        }
        case CXCursor_EnumDecl: {
            spdlog::debug("Found enum declaration: {}", cursor_name_str);

            visit_if_cursor_valid(
                cursor, [ctx, is_struct, cursor_name_str](CXCursor cursor) {
                    enum_ e{};
                    e.name = cursor_name_str;
                    e.namespace_ = ctx->namespace_;

                    clang_visitChildren(cursor, enum_visitor, &e);

                    ctx->diagram.enums.emplace_back(std::move(e));
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
