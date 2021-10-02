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

#include "config/config.h"
#include "cx/cursor.h"
#include "uml/class_diagram/model/diagram.h"

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <cppast/cpp_friend.hpp>
#include <cppast/cpp_function_template.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <cppast/cpp_template_parameter.hpp>
#include <cppast/cpp_type.hpp>
#include <cppast/visitor.hpp>
#include <type_safe/reference.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml {
namespace visitor {
namespace class_diagram {

struct tu_context {
    tu_context(cppast::cpp_entity_index &idx,
        clanguml::class_diagram::model::diagram &d_,
        const clanguml::config::class_diagram &config_);

    bool has_type_alias(const std::string &full_name) const;

    void add_type_alias(const std::string &full_name,
        type_safe::object_ref<const cppast::cpp_type> &&ref);

    type_safe::object_ref<const cppast::cpp_type> get_type_alias(
        const std::string &full_name) const;

    type_safe::object_ref<const cppast::cpp_type> get_type_alias_final(
        const cppast::cpp_type &t) const;

    bool has_type_alias_template(const std::string &full_name) const;

    void add_type_alias_template(const std::string &full_name,
        type_safe::object_ref<const cppast::cpp_type> &&ref);

    type_safe::object_ref<const cppast::cpp_type> get_type_alias_template(
        const std::string &full_name) const;

    std::vector<std::string> namespace_;
    cppast::cpp_entity_index &entity_index;
    clanguml::class_diagram::model::diagram &d;
    const clanguml::config::class_diagram &config;
    std::map<std::string, type_safe::object_ref<const cppast::cpp_type>>
        alias_index;
    std::map<std::string, type_safe::object_ref<const cppast::cpp_type>>
        alias_template_index;
};

template <typename T> struct element_visitor_context {
    element_visitor_context(clanguml::class_diagram::model::diagram &d_, T &e);

    tu_context *ctx;

    T &element;
    clanguml::class_diagram::model::class_ *parent_class{};
    clanguml::class_diagram::model::diagram &d;
};

class tu_visitor {
public:
    tu_visitor(cppast::cpp_entity_index &idx_,
        clanguml::class_diagram::model::diagram &d_,
        const clanguml::config::class_diagram &config_);

    void operator()(const cppast::cpp_entity &file);

    void process_class_declaration(const cppast::cpp_class &cls,
        type_safe::optional_ref<const cppast::cpp_template_specialization>
            tspec = nullptr);

    void process_enum_declaration(const cppast::cpp_enum &enm);

    void process_anonymous_enum(const cppast::cpp_enum &en,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_field(const cppast::cpp_member_variable &mv,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    bool process_field_with_template_instantiation(
        const cppast::cpp_member_variable &mv, const cppast::cpp_type &tr,
        clanguml::class_diagram::model::class_ &c,
        clanguml::class_diagram::model::class_member &m,
        cppast::cpp_access_specifier_kind as);

    void process_static_field(const cppast::cpp_variable &mv,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_method(const cppast::cpp_member_function &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_template_method(const cppast::cpp_function_template &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_static_method(const cppast::cpp_function &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_constructor(const cppast::cpp_constructor &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_destructor(const cppast::cpp_destructor &mf,
        clanguml::class_diagram::model::class_ &c,
        cppast::cpp_access_specifier_kind as);

    void process_function_parameter(const cppast::cpp_function_parameter &param,
        clanguml::class_diagram::model::class_method &m,
        clanguml::class_diagram::model::class_ &c,
        const std::set<std::string> &template_parameter_names = {});

    bool find_relationships(const cppast::cpp_type &t,
        std::vector<std::pair<std::string,
            clanguml::class_diagram::model::relationship_t>> &relationships,
        clanguml::class_diagram::model::relationship_t relationship_hint =
            clanguml::class_diagram::model::relationship_t::kNone);

    void process_template_type_parameter(
        const cppast::cpp_template_type_parameter &t,
        clanguml::class_diagram::model::class_ &parent);

    void process_template_nontype_parameter(
        const cppast::cpp_non_type_template_parameter &t,
        clanguml::class_diagram::model::class_ &parent);

    void process_template_template_parameter(
        const cppast::cpp_template_template_parameter &t,
        clanguml::class_diagram::model::class_ &parent);

    void process_friend(const cppast::cpp_friend &t,
        clanguml::class_diagram::model::class_ &parent);

private:
    clanguml::class_diagram::model::class_ build_template_instantiation(
        const cppast::cpp_template_instantiation_type &t,
        std::optional<clanguml::class_diagram::model::class_ *> parent = {});

    const cppast::cpp_type &resolve_alias(const cppast::cpp_type &t);

    tu_context ctx;
};
}
}
}
