/**
 * src/include_diagram/visitor/translation_unit_visitor.cc
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

#include "translation_unit_visitor.h"

#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_preprocessor.hpp>

namespace clanguml::include_diagram::visitor {

using clanguml::include_diagram::model::diagram;

translation_unit_visitor::translation_unit_visitor(
    cppast::cpp_entity_index &idx,
    clanguml::include_diagram::model::diagram &diagram,
    const clanguml::config::include_diagram &config)
    : ctx{idx, diagram, config}
{
}

void translation_unit_visitor::operator()(const cppast::cpp_entity &file)
{
    cppast::visit(file,
        [&, this](const cppast::cpp_entity &e, cppast::visitor_info info) {
            if (e.kind() == cppast::cpp_entity_kind::include_directive_t) {

                const auto &inc =
                    static_cast<const cppast::cpp_include_directive &>(e);

                auto file_name = std::filesystem::path(inc.full_path());

                auto f = std::make_unique<model::source_file>();
                f->set_path(file_name.parent_path().string());
                f->set_name(file_name.filename().string());
            }
        });
}
}
