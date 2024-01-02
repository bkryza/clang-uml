/**
 * @file src/common/visitor/comment/plain_visitor.h
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

#include <clang/AST/ASTContext.h>
#include <clang/AST/Comment.h>
#include <clang/Basic/SourceManager.h>

#include "comment_visitor.h"

namespace clanguml::common::visitor::comment {

/**
 * @brief Plain comment visitor which extracts raw and formatted comment.
 */
class plain_visitor : public comment_visitor {
public:
    plain_visitor(clang::SourceManager &source_manager);

    /**
     * Extracts 'raw' and 'formatted' comment values from the Clang's
     * parser.
     *
     * @param decl Clang's named declaration
     * @param e Diagram element
     */
    void visit(const clang::NamedDecl &decl,
        common::model::decorated_element &e) override;
};

} // namespace clanguml::common::visitor::comment