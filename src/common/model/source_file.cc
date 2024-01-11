/**
 * @file src/common/model/source_file.cc
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

#include "source_file.h"

namespace clanguml::common::model {

std::string to_string(source_file_t sf)
{
    switch (sf) {
    case source_file_t::kDirectory:
        return "directory";
    case source_file_t::kHeader:
        return "header";
    case source_file_t::kImplementation:
        return "implementation";
    default:
        assert(false);
        return "";
    }
}

} // namespace clanguml::common::model
