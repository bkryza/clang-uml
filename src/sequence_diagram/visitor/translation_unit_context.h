/**
 * src/sequence_diagram/visitor/translation_unit_context.h
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
#include "sequence_diagram/model/diagram.h"

#include <cppast/cpp_entity_index.hpp>

#include <functional>
#include <memory>
#include <string>

namespace clanguml::sequence_diagram::visitor {

class translation_unit_context {
public:
    translation_unit_context(cppast::cpp_entity_index &idx,
        clanguml::sequence_diagram::model::diagram &diagram,
        const clanguml::config::sequence_diagram &config);

    void push_namespace(const std::string &ns);

    void pop_namespace();

    const std::vector<std::string> &get_namespace() const;

    const cppast::cpp_entity_index &entity_index() const;

    const clanguml::config::sequence_diagram &config() const;

    clanguml::sequence_diagram::model::diagram &diagram();

    void set_current_method(cx::cursor method);

    cx::cursor &current_method();

private:
    // Reference to the cppast entity index
    cppast::cpp_entity_index &entity_index_;
    clanguml::sequence_diagram::model::diagram &diagram_;
    const clanguml::config::sequence_diagram &config_;

    std::vector<std::string> namespace_;
    cx::cursor current_method_;
};

}
