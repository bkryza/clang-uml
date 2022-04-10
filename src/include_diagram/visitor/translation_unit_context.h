/**
 * src/include_diagram/visitor/translation_unit_context.h
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

#include "common/model/package.h"
#include "config/config.h"
#include "include_diagram/model/diagram.h"

#include <cppast/cpp_entity_index.hpp>
#include <cppast/cpp_namespace.hpp>
#include <cppast/cpp_type.hpp>
#include <type_safe/reference.hpp>

namespace clanguml::include_diagram::visitor {

class translation_unit_context {
public:
    translation_unit_context(cppast::cpp_entity_index &idx,
        clanguml::include_diagram::model::diagram &diagram,
        const clanguml::config::include_diagram &config);

    const cppast::cpp_entity_index &entity_index() const;

    const clanguml::config::include_diagram &config() const;

    clanguml::include_diagram::model::diagram &diagram();

    void set_current_file(
        type_safe::optional_ref<common::model::source_file> p);

    type_safe::optional_ref<common::model::source_file>
    get_current_file() const;

private:
    // Reference to the cppast entity index
    cppast::cpp_entity_index &entity_index_;

    // Reference to the output diagram model
    clanguml::include_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::include_diagram &config_;

    type_safe::optional_ref<common::model::source_file> current_file_;
};

}
