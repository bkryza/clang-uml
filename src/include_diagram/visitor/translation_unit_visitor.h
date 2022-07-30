/**
 * src/include_diagram/visitor/translation_unit_visitor.h
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

#include "common/model/enums.h"
#include "common/model/package.h"
#include "config/config.h"
#include "include_diagram/model/diagram.h"
#include "include_diagram/visitor/translation_unit_context.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::include_diagram::visitor {

class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor> {
public:
    translation_unit_visitor(clang::SourceManager &sm,
        clanguml::include_diagram::model::diagram &diagram,
        const clanguml::config::include_diagram &config);

    void operator()(const cppast::cpp_entity &file);

    void finalize() { }

private:
    clang::SourceManager &source_manager_;

    // Reference to the output diagram model
    clanguml::include_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::include_diagram &config_;
};
}
