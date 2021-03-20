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
#include "config/config.h"
#include "cx/cursor.h"

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include <functional>
#include <memory>
#include <string>

namespace clanguml {
namespace visitor {
namespace class_diagram {

struct tu_context {
    tu_context(clanguml::model::class_diagram::diagram &d_,
        const clanguml::config::class_diagram &config_)
        : d{d_}
        , config{config_}
    {
    }

    std::vector<std::string> namespace_;
    clanguml::model::class_diagram::diagram &d;
    const clanguml::config::class_diagram &config;
};

template <typename T> struct element_visitor_context {
    element_visitor_context(clanguml::model::class_diagram::diagram &d_, T &e)
        : element(e)
        , d{d_}
    {
    }
    tu_context *ctx;

    T &element;
    clanguml::model::class_diagram::class_ *parent_class{};
    clanguml::model::class_diagram::diagram &d;
};

// Visitors

enum CXChildVisitResult visit_if_cursor_valid(
    cx::cursor cursor, std::function<enum CXChildVisitResult(cx::cursor)> f);

enum CXChildVisitResult enum_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data);

enum CXChildVisitResult method_parameter_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data);

enum CXChildVisitResult friend_class_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data);

enum CXChildVisitResult class_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data);

enum CXChildVisitResult translation_unit_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data);

// Entity processors

enum CXChildVisitResult process_class_base_specifier(cx::cursor cursor,
    clanguml::model::class_diagram::class_ *parent, struct tu_context *ctx);

enum CXChildVisitResult process_template_type_parameter(cx::cursor cursor,
    clanguml::model::class_diagram::class_ *parent, struct tu_context *ctx);

enum CXChildVisitResult process_template_nontype_parameter(cx::cursor cursor,
    clanguml::model::class_diagram::class_ *parent, struct tu_context *ctx);

enum CXChildVisitResult process_template_template_parameter(cx::cursor cursor,
    clanguml::model::class_diagram::class_ *parent, struct tu_context *ctx);

enum CXChildVisitResult process_method(cx::cursor cursor,
    clanguml::model::class_diagram::class_ *parent, struct tu_context *ctx);

enum CXChildVisitResult process_class_declaration(cx::cursor cursor,
    bool is_struct, clanguml::model::class_diagram::class_ *parent,
    struct tu_context *ctx);

enum CXChildVisitResult process_enum_declaration(cx::cursor cursor,
    clanguml::model::class_diagram::class_ *parent, struct tu_context *ctx);

bool process_template_specialization_class_field(cx::cursor cursor, cx::type t,
    clanguml::model::class_diagram::class_ *parent, struct tu_context *ctx);

enum CXChildVisitResult process_field(cx::cursor cursor,
    clanguml::model::class_diagram::class_ *parent, struct tu_context *ctx);

// Utils

clanguml::model::class_diagram::class_ build_template_instantiation(
    cx::cursor cursor, cx::type t);

clanguml::model::class_diagram::scope_t cx_access_specifier_to_scope(
    CX_CXXAccessSpecifier as);

void find_relationships(cx::type t,
    std::vector<std::pair<cx::type,
        clanguml::model::class_diagram::relationship_t>> &relationships,
    clanguml::model::class_diagram::relationship_t relationship_hint =
        clanguml::model::class_diagram::relationship_t::kNone);
}
}
}
