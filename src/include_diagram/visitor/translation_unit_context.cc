/**
 * src/include_diagram/visitor/translation_unit_context.cc
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

#include "translation_unit_context.h"

#include "cx/util.h"

namespace clanguml::include_diagram::visitor {

translation_unit_context::translation_unit_context(
    cppast::cpp_entity_index &idx,
    clanguml::include_diagram::model::diagram &diagram,
    const clanguml::config::include_diagram &config)
    : entity_index_{idx}
    , diagram_{diagram}
    , config_{config}
{
}

const cppast::cpp_entity_index &translation_unit_context::entity_index() const
{
    return entity_index_;
}

const clanguml::config::include_diagram &
translation_unit_context::config() const
{
    return config_;
}

clanguml::include_diagram::model::diagram &translation_unit_context::diagram()
{
    return diagram_;
}

void translation_unit_context::set_current_file(
    type_safe::optional_ref<common::model::source_file> f)
{
    current_file_ = f;
}

type_safe::optional_ref<common::model::source_file>
translation_unit_context::get_current_file() const
{
    return current_file_;
}

}
