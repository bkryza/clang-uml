/**
 * src/package_diagram/visitor/translation_unit_visitor.h
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

#include "config/config.h"
#include "package_diagram/model/diagram.h"
#include "package_diagram/visitor/translation_unit_context.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include <common/model/enums.h>
#include <common/model/package.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::package_diagram::visitor {

class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor>{
public:
    translation_unit_visitor(clang::SourceManager &sm,
        clanguml::package_diagram::model::diagram &diagram,
        const clanguml::config::package_diagram &config);

    void finalize() {}
private:
    clang::SourceManager &source_manager_;

    // Reference to the output diagram model
    clanguml::package_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::package_diagram &config_;
};
}
