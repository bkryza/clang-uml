/**
 * @file src/common/visitor/ast_id_mapper.cc
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

#include "ast_id_mapper.h"

namespace clanguml::common::visitor {

void ast_id_mapper::add(int64_t ast_id, eid_t global_id)
{
    id_map_.emplace(ast_id, global_id);
}

std::optional<eid_t> ast_id_mapper::get_global_id(eid_t ast_id)
{
    assert(!ast_id.is_global());

    if (ast_id.is_global())
        return {};

    if (id_map_.count(ast_id.ast_local_value()) == 0)
        return {};

    return id_map_.at(ast_id.ast_local_value());
}

} // namespace clanguml::common::visitor