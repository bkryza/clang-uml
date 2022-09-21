/**
 * src/common/visitor/comment/comment_visitor.h
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

#include <clang/AST/Comment.h>
#include <clang/Basic/SourceManager.h>

#include "common/model/decorated_element.h"

namespace clanguml::common::visitor::comment {

class comment_visitor {
public:
    comment_visitor(clang::SourceManager &source_manager);

    virtual ~comment_visitor() = default;

    virtual void visit(
        const clang::NamedDecl &decl, common::model::decorated_element &e) = 0;

    clang::SourceManager &source_manager();

private:
    clang::SourceManager &source_manager_;
};
}