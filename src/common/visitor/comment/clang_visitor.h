/**
 * @file src/common/visitor/comment/clang_visitor.h
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
 * @brief Uses Clang's comment parser to extract Doxygen-style comment blocks.
 */
class clang_visitor : public comment_visitor {
public:
    clang_visitor(clang::SourceManager &source_manager);

    /**
     * Extracts Doxygen style comment blocks from the comment.
     *
     * @param decl Clang's named declaration
     * @param e Diagram element
     */
    void visit(const clang::NamedDecl &decl,
        common::model::decorated_element &e) override;

private:
    void visit_block_command(
        const clang::comments::BlockCommandComment *command,
        const clang::comments::CommandTraits &traits,
        common::model::comment_t &cmt);

    void visit_param_command(
        const clang::comments::ParamCommandComment *command,
        const clang::comments::CommandTraits &traits,
        common::model::comment_t &cmt);

    void visit_tparam_command(
        const clang::comments::TParamCommandComment *command,
        const clang::comments::CommandTraits &traits,
        common::model::comment_t &cmt);

    void visit_paragraph(const clang::comments::ParagraphComment *paragraph,
        const clang::comments::CommandTraits &traits, std::string &text);
};
} // namespace clanguml::common::visitor::comment