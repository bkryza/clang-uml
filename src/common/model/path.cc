/**
 * @file src/common/model/path.cc
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

#include "path.h"

namespace clanguml::common::model {

std::string to_string(const path_type pt)
{
    switch (pt) {
    case path_type::kModule:
        return "module";
    case path_type::kNamespace:
        return "namespace";
    case path_type::kFilesystem:
        return "directory";
    default:
        assert(false);
        return "";
    }
}

} // namespace clanguml::common::model