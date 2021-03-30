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
#include <cppast/cpp_friend.hpp>
#include <cppast/cpp_function_template.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <cppast/cpp_template_parameter.hpp>
#include <cppast/cpp_type.hpp>
#include <cppast/visitor.hpp>

#include <functional>
#include <memory>
#include <string>

namespace clanguml {
namespace visitor {
namespace class_diagram {

struct tu_context {
    tu_context(cppast::cpp_entity_index &idx,
        clanguml::model::class_diagram::diagram &d_,
        const clanguml::config::class_diagram &config_)
        : entity_index{idx}
        , d{d_}
        , config{config_}
    {
    }

    std::vector<std::string> namespace_;
    cppast::cpp_entity_index &entity_index;
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

class tu_visitor {
public:
    tu_visitor(cppast::cpp_entity_index &idx_,
        clanguml::model::class_diagram::diagram &d_,
        const clanguml::config::class_diagram &config_)
        : ctx{idx_, d_, config_}
    {
    }

    void operator()(const cppast::cpp_entity &file);

    void process_class_declaration(const cppast::cpp_class &cls);

    void process_enum_declaration(const cppast::cpp_enum &enm);

    void process_field(const cppast::cpp_member_variable &mv,
        clanguml::model::class_diagram::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_static_field(const cppast::cpp_variable &mv,
        clanguml::model::class_diagram::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_method(const cppast::cpp_member_function &mf,
        clanguml::model::class_diagram::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_template_method(const cppast::cpp_function_template &mf,
        clanguml::model::class_diagram::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_static_method(const cppast::cpp_function &mf,
        clanguml::model::class_diagram::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_constructor(const cppast::cpp_constructor &mf,
        clanguml::model::class_diagram::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_destructor(const cppast::cpp_destructor &mf,
        clanguml::model::class_diagram::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_function_parameter(const cppast::cpp_function_parameter &param,
        clanguml::model::class_diagram::class_method &m,
        clanguml::model::class_diagram::class_ &c);

    void find_relationships(const cppast::cpp_type &t,
        std::vector<std::pair<std::string,
            clanguml::model::class_diagram::relationship_t>> &relationships,
        clanguml::model::class_diagram::relationship_t relationship_hint =
            clanguml::model::class_diagram::relationship_t::kNone);

    void process_template_type_parameter(
        const cppast::cpp_template_type_parameter &t,
        clanguml::model::class_diagram::class_ &parent);

    void process_template_nontype_parameter(
        const cppast::cpp_non_type_template_parameter &t,
        clanguml::model::class_diagram::class_ &parent);

    void process_template_template_parameter(
        const cppast::cpp_template_template_parameter &t,
        clanguml::model::class_diagram::class_ &parent);

    void process_friend(const cppast::cpp_friend &t,
        clanguml::model::class_diagram::class_ &parent);

private:
    clanguml::model::class_diagram::class_ build_template_instantiation(
        const cppast::cpp_entity &e,
        const cppast::cpp_template_instantiation_type &t);

    tu_context ctx;
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
