/**
 * @file docs/architecture.h
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

#include "class_diagram/model/diagram.h"
#include "common/model/diagram.h"
#include "include_diagram/model/diagram.h"
#include "package_diagram/model/diagram.h"
#include "sequence_diagram/model/diagram.h"

#include "class_diagram/visitor/translation_unit_visitor.h"
#include "common/visitor/translation_unit_visitor.h"
#include "include_diagram/visitor/translation_unit_visitor.h"
#include "package_diagram/visitor/translation_unit_visitor.h"
#include "sequence_diagram/visitor/translation_unit_visitor.h"

#include "class_diagram/generators/json/class_diagram_generator.h"
#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "common/generators/generators.h"

/*
 * This file serves as an example how high-level documentation can be stored
 * directly in the code.
 */

namespace clanguml {
/**
 * This namespace provides common interfaces for all kinds of diagrams.
 *
 * The core diagram functionality is divided into 3 groups: visitor, model
 * and generators.
 */
namespace common {
/**
 * This namespace provides common interfaces for translation unit visitors,
 * which are responsible for traversing the Clang's AST of the source code,
 * and generating the intermedia diagram model.
 *
 * Each 'translation_unit_visitor' implements the Clang's
 * 'RecursiveASTVisitor' interface.
 */
namespace visitor {
} // namespace visitor
/**
 * This namespace provides common interfaces for diagram model, including
 * various diagram elements and diagram filters.
 */
namespace model {

} // namespace model
/**
 *
 */
namespace generators {

} // namespace generators
} // namespace common
} // namespace clanguml