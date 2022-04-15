/**
 * src/package_diagram/visitor/translation_unit_visitor.h
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

#include "config/config.h"
#include "cx/cursor.h"
#include "package_diagram/model/diagram.h"
#include "package_diagram/visitor/translation_unit_context.h"

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

#include <common/model/enums.h>
#include <common/model/package.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::package_diagram::visitor {

class translation_unit_visitor {
public:
    translation_unit_visitor(cppast::cpp_entity_index &idx,
        clanguml::package_diagram::model::diagram &diagram,
        const clanguml::config::package_diagram &config);

    void operator()(const cppast::cpp_entity &file);

    void process_class_declaration(const cppast::cpp_class &cls,
        type_safe::optional_ref<const cppast::cpp_template_specialization>
            tspec = nullptr);

    void process_function(
        const cppast::cpp_function &f, bool skip_return_type = false);

    bool find_relationships(const cppast::cpp_type &t_,
        std::vector<std::pair<std::string, common::model::relationship_t>>
            &relationships,
        common::model::relationship_t relationship_hint);

private:
    /**
     * Try to resolve a type instance into a type referenced through an alias.
     * If t does not represent an alias, returns t.
     */
    const cppast::cpp_type &resolve_alias(const cppast::cpp_type &t);

    // ctx allows to track current visitor context, e.g. current namespace
    translation_unit_context ctx;
};
}
