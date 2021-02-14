#pragma once

#include "cx.h"
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

using clanguml::model::sequence_diagram::diagram;
using clanguml::model::sequence_diagram::message;
using clanguml::model::sequence_diagram::message_t;

struct tu_context {
    tu_context(diagram &d_)
        : d{d_}
    {
    }

    std::vector<std::string> namespace_;
    cx::cursor current_method;
    clanguml::model::sequence_diagram::diagram &d;
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

            if (referenced.kind() == CXCursor_CXXMethod) {
                if (sp_name.find("clanguml::") == 0) {
                    // Get calling object
                    std::string caller =
                        ctx->current_method.semantic_parent().fully_qualified();
                    if (caller.empty() &&
                        clang_Location_isFromMainFile(cursor.location()) == 0)
                        caller = "<MAIN>";

                    std::string caller_usr = ctx->current_method.usr();
                    // Get called object
                    std::string callee =
                        cursor.referenced().semantic_parent().fully_qualified();

                    // Get called method
                    std::string called_message = cursor.spelling();

                    // Found method call: CXCursorKind () const
                    spdlog::debug("Found method call at line {}:{} "
                                  "\n\tCURRENT_METHOD: {}\n\tFROM: {}\n\tTO: "
                                  "{}\n\tMESSAGE: {}",
                        clang_getCString(clang_getFileName(f)), line,
                        ctx->current_method.spelling(), caller, callee,
                        called_message);

                    message m;
                    m.type = message_t::kCall;
                    m.from = caller;
                    m.from_usr = caller_usr;
                    m.line = line;
                    m.to = callee;
                    m.message = called_message;

                    ctx->d.sequence.emplace_back(std::move(m));
                }
            }
            else if (referenced.kind() == CXCursor_FunctionDecl) {
                // TODO
            }

            ret = CXChildVisit_Continue;
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
