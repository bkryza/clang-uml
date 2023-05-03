/**
 * src/class_diagram/visitor/ast_id_mapper.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

#include "common/model/diagram_element.h"

#include <cstdint>
#include <map>

namespace clanguml::common::visitor {

class ast_id_mapper {
public:
    using id_t = common::model::diagram_element::id_t;

    ast_id_mapper() = default;

    void add(int64_t ast_id, id_t global_id);

    std::optional<id_t> get_global_id(int64_t ast_id);

private:
    std::map</* Clang AST translation unit local id */ int64_t,
        /* clang-uml global id */ id_t>
        id_map_;
};

} // namespace clanguml::common::visitor