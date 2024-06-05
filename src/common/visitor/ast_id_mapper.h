/**
 * @file src/common/visitor/ast_id_mapper.h
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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

using clanguml::common::eid_t;

/**
 * @brief Mapping between Clang AST identifier and `clang-uml` unique ids
 *
 * Since identifiers provided by Clang AST are not transferable between
 * translation units (i.e. the same class can have a different id when visited
 * from different translation units), we need global identifiers which are
 * the same among all translation units.
 *
 * Currently they are calculated as hashes from the fully qualified string
 * representation of the type.
 *
 * This class allows to store mappings between Clang local identifiers
 * in current translation unit and the global identifiers for the element.
 */
class ast_id_mapper {
public:
    ast_id_mapper() = default;

    /**
     * Add id mapping.
     *
     * @param ast_id Clang's local AST id.
     * @param global_id Global element id.
     */
    void add(int64_t ast_id, eid_t global_id);

    /**
     * Get global element id based on it's local Clang AST id, if exists.
     *
     * @param ast_id Clang's local AST id.
     * @return Global id, if exists.
     */
    std::optional<eid_t> get_global_id(eid_t ast_id);

private:
    std::map</* Clang AST translation unit local id */ int64_t,
        /* clang-uml global id */ eid_t>
        id_map_;
};

} // namespace clanguml::common::visitor