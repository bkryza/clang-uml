#pragma once

#include "cx/cursor.h"
#include "sequence_diagram_model.h"

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>

namespace clanguml {
namespace visitor {
namespace sequence_diagram {

using clanguml::model::sequence_diagram::activity;
using clanguml::model::sequence_diagram::diagram;
using clanguml::model::sequence_diagram::message;
using clanguml::model::sequence_diagram::message_t;

struct tu_context {
    tu_context(diagram &d_, const clanguml::config::sequence_diagram &config_)
        : d{d_}
        , config{config_}
    {
    }

    std::vector<std::string> namespace_;
    cx::cursor current_method;
    clanguml::model::sequence_diagram::diagram &d;
    const clanguml::config::sequence_diagram &config;
};

static enum CXChildVisitResult translation_unit_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data)
{
    struct tu_context *ctx = (struct tu_context *)client_data;

    enum CXChildVisitResult ret = CXChildVisit_Break;

    cx::cursor cursor{std::move(cx_cursor)};
    cx::cursor parent{std::move(cx_parent)};

    if (cursor.spelling().empty()) {
        return CXChildVisit_Recurse;
    }

    switch (cursor.kind()) {
        case CXCursor_FunctionTemplate:
        case CXCursor_CXXMethod:
        case CXCursor_FunctionDecl:
            ctx->current_method = cursor;
            ret = CXChildVisit_Recurse;
            break;
        case CXCursor_CallExpr: {
            auto referenced = cursor.referenced();
            auto referenced_type = referenced.type();
            auto referenced_cursor_name = referenced.display_name();

            auto semantic_parent = referenced.semantic_parent();
            auto sp_name = semantic_parent.fully_qualified();
            auto lexical_parent = cursor.lexical_parent();
            auto lp_name = lexical_parent.spelling();

            CXFile f;
            unsigned int line{};
            unsigned int column{};
            unsigned int offset{};
            clang_getFileLocation(
                cursor.location(), &f, &line, &column, &offset);
            std::string file{clang_getCString(clang_getFileName(f))};

            auto &d = ctx->d;
            auto &config = ctx->config;
            if (referenced.kind() == CXCursor_CXXMethod) {
                if (config.should_include(sp_name)) {
                    // Get calling object
                    std::string caller{};
                    if (ctx->current_method.semantic_parent()
                            .is_translation_unit() ||
                        ctx->current_method.semantic_parent().is_namespace()) {
                        caller = ctx->current_method.semantic_parent()
                                     .fully_qualified() +
                            "::" + ctx->current_method.spelling() + "()";
                    }
                    else {
                        caller = ctx->current_method.semantic_parent()
                                     .fully_qualified();
                    }

                    auto caller_usr = ctx->current_method.usr();
                    // Get called object
                    auto callee =
                        referenced.semantic_parent().fully_qualified();
                    auto callee_usr = referenced.semantic_parent().usr();

                    // Get called method
                    auto called_message = cursor.spelling();

                    // Found method call: CXCursorKind () const
                    spdlog::debug(
                        "Adding method call at line {}:{} to diagram {}"
                        "\n\tCURRENT_METHOD: {}\n\tFROM: '{}'\n\tTO: "
                        "{}\n\tMESSAGE: {}\n\tFROM_USR: {}\n\tTO_USR: "
                        "{}\n\tRETURN_TYPE: {}",
                        file, line, d.name, ctx->current_method.spelling(),
                        caller, callee, called_message, caller_usr, callee_usr,
                        referenced.type().result_type().spelling());

                    message m;
                    m.type = message_t::kCall;
                    m.from = caller;
                    m.from_usr = caller_usr;
                    m.line = line;
                    m.to = callee;
                    m.to_usr = referenced.usr();
                    m.message = called_message;
                    m.return_type = referenced.type().result_type().spelling();

                    if (d.sequences.find(caller_usr) == d.sequences.end()) {
                        activity a;
                        a.usr = caller_usr;
                        a.from = caller;
                        d.sequences.insert({caller_usr, std::move(a)});
                    }

                    d.sequences[caller_usr].messages.emplace_back(std::move(m));
                }
            }
            else if (referenced.kind() == CXCursor_FunctionDecl) {
                // TODO
            }

            ret = CXChildVisit_Recurse;
            break;
        }
        case CXCursor_Namespace: {
            ret = CXChildVisit_Recurse;
            break;
        }
        default:
            ret = CXChildVisit_Recurse;
    }

    return ret;
}
}
}
}
