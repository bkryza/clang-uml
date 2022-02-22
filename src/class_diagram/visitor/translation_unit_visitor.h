/**
 * src/class_diagram/visitor/translation_unit_visitor.h
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
#pragma once

#include "class_diagram/model/diagram.h"
#include "class_diagram/visitor/translation_unit_context.h"
#include "config/config.h"
#include "cx/cursor.h"

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <cppast/cpp_friend.hpp>
#include <cppast/cpp_function_template.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <cppast/cpp_template.hpp>
#include <cppast/cpp_template_parameter.hpp>
#include <cppast/cpp_type.hpp>
#include <cppast/visitor.hpp>
#include <type_safe/reference.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::class_diagram::visitor {

class translation_unit_visitor {
public:
    translation_unit_visitor(cppast::cpp_entity_index &idx,
        clanguml::class_diagram::model::diagram &diagram,
        const clanguml::config::class_diagram &config);

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
            clanguml::common::model::relationship_t>> &relationships,
        clanguml::common::model::relationship_t relationship_hint =
            clanguml::common::model::relationship_t::kNone);

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
    std::unique_ptr<clanguml::class_diagram::model::class_>
    build_template_instantiation(
        const cppast::cpp_template_instantiation_type &t,
        std::optional<clanguml::class_diagram::model::class_ *> parent = {});

    /**
     * Try to resolve a type instance into a type referenced through an alias.
     * If t does not represent an alias, returns t.
     */
    const cppast::cpp_type &resolve_alias(const cppast::cpp_type &t);
    const cppast::cpp_type &resolve_alias_template(
        const cppast::cpp_type &type);

    // ctx allows to track current visitor context, e.g. current namespace
    translation_unit_context ctx;
};
}
