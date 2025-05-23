/**
 * @file src/sequence_diagram/model/jinja_context.h
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
#pragma once

#include "common/model/jinja_context.h"
#include "sequence_diagram/model/diagram.h"

#include <string>

namespace clanguml::common::jinja {

void to_json(inja::json &ctx,
    const diagram_context<sequence_diagram::model::diagram> &d);

} // namespace clanguml::common::jinja