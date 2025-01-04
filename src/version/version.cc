/**
 * src/version/version.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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
#include "version-const.h"

namespace clanguml::version {
const char *version() { return CLANG_UML_VERSION; }

unsigned json_generator_schema_version()
{
    return CLANG_UML_JSON_GENERATOR_SCHEMA_VERSION;
}
} // namespace clanguml::version