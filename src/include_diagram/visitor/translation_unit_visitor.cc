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

#include <filesystem>

namespace clanguml::include_diagram::visitor {

translation_unit_visitor::translation_unit_visitor(clang::SourceManager &sm,
    clanguml::include_diagram::model::diagram &diagram,
    const clanguml::config::include_diagram &config)
    : source_manager_{sm}
    , diagram_{diagram}
    , config_{config}
{
}

}
