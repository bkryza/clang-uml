/**
 * @file src/common/visitor/comment/clang_visitor.cc
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

#include "clang_visitor.h"
#include "util/util.h"

#if LLVM_VERSION_MAJOR > 17
#define CLANG_UML_LLVM_COMMENT_KIND(COMMENT_KIND)                              \
    clang::comments::CommentKind::COMMENT_KIND
#else
#define CLANG_UML_LLVM_COMMENT_KIND(COMMENT_KIND)                              \
    clang::comments::Comment::COMMENT_KIND##Kind
#endif

namespace clanguml::common::visitor::comment {

clang_visitor::clang_visitor(clang::SourceManager &source_manager)
    : comment_visitor{source_manager}
{
}

void clang_visitor::visit(
    const clang::NamedDecl &decl, common::model::decorated_element &e)
{
    const auto *comment =
        decl.getASTContext().getRawCommentForDeclNoCache(&decl);

    if (comment == nullptr) {
        return;
    }

    auto raw_comment = comment->getRawText(source_manager());

    auto formatted_comment = comment->getFormattedText(
        source_manager(), decl.getASTContext().getDiagnostics());

    common::model::comment_t cmt = inja::json::object();
    cmt["raw"] = raw_comment;
    cmt["formatted"] = formatted_comment;

    using clang::comments::BlockCommandComment;
    using clang::comments::FullComment;
    using clang::comments::ParagraphComment;
    using clang::comments::ParamCommandComment;
    using clang::comments::TextComment;
    using clang::comments::TParamCommandComment;

    FullComment *full_comment =
        comment->parse(decl.getASTContext(), nullptr, &decl);

    const auto &traits = decl.getASTContext().getCommentCommandTraits();

    for (const auto *block : full_comment->getBlocks()) {
        const auto block_kind = block->getCommentKind();
        if (block_kind == CLANG_UML_LLVM_COMMENT_KIND(ParagraphComment)) {
            std::string paragraph_text;

            visit_paragraph(clang::dyn_cast<ParagraphComment>(block), traits,
                paragraph_text);
            if (!cmt.contains("text"))
                cmt["text"] = "";

            cmt["text"] =
                cmt["text"].get<std::string>() + "\n" + paragraph_text;

            if (!cmt.contains("paragraph"))
                cmt["paragraph"] = inja::json::array();

            cmt["paragraph"].push_back(paragraph_text);
        }
        else if (block_kind == CLANG_UML_LLVM_COMMENT_KIND(TextComment)) {
            // TODO
        }
        else if (block_kind ==
            CLANG_UML_LLVM_COMMENT_KIND(ParamCommandComment)) {
            visit_param_command(
                clang::dyn_cast<ParamCommandComment>(block), traits, cmt);
        }
        else if (block_kind ==
            CLANG_UML_LLVM_COMMENT_KIND(TParamCommandComment)) {
            visit_tparam_command(
                clang::dyn_cast<TParamCommandComment>(block), traits, cmt);
        }
        else if (block_kind ==
            CLANG_UML_LLVM_COMMENT_KIND(BlockCommandComment)) {
            if (const auto *command =
                    clang::dyn_cast<BlockCommandComment>(block);
                command != nullptr) {
                const auto *command_info =
                    traits.getCommandInfo(command->getCommandID());

                if (command_info->IsBlockCommand &&
                    command_info->NumArgs == 0U) {
                    // Visit block command with a single text argument, e.g.:
                    //    \brief text
                    //    \todo text
                    //    ...
                    visit_block_command(command, traits, cmt);
                }
                else if (command_info->IsParamCommand) {
                    // Visit function param block:
                    //   \param arg text
                    visit_param_command(
                        clang::dyn_cast<ParamCommandComment>(command), traits,
                        cmt);
                }
                else if (command_info->IsTParamCommand) {
                    // Visit template param block:
                    //   \tparam typename text
                    visit_tparam_command(
                        clang::dyn_cast<TParamCommandComment>(command), traits,
                        cmt);
                }
            }
        }
    }
    e.set_comment(cmt);
}

void clang_visitor::visit_block_command(
    const clang::comments::BlockCommandComment *command,
    const clang::comments::CommandTraits &traits, common::model::comment_t &cmt)
{
    using clang::comments::Comment;
    using clang::comments::ParagraphComment;
    using clang::comments::TextComment;

    std::string command_text;

    for (const auto *paragraph_it = command->child_begin();
         paragraph_it != command->child_end(); ++paragraph_it) {

        if ((*paragraph_it)->getCommentKind() ==
            CLANG_UML_LLVM_COMMENT_KIND(ParagraphComment)) {
            visit_paragraph(clang::dyn_cast<ParagraphComment>(*paragraph_it),
                traits, command_text);
        }
    }

    const auto command_name = command->getCommandName(traits).str();
    if (!command_text.empty()) {
        if (!cmt.contains(command_name))
            cmt[command_name] = inja::json::array();

        cmt[command_name].push_back(command_text);
    }
}

void clang_visitor::visit_param_command(
    const clang::comments::ParamCommandComment *command,
    const clang::comments::CommandTraits &traits, common::model::comment_t &cmt)
{
    using clang::comments::Comment;
    using clang::comments::ParagraphComment;
    using clang::comments::TextComment;

    std::string description;

    if (command == nullptr)
        return;

    const auto name = command->getParamNameAsWritten().str();

    for (const auto *it = command->child_begin(); it != command->child_end();
         ++it) {

        if ((*it)->getCommentKind() ==
            CLANG_UML_LLVM_COMMENT_KIND(ParagraphComment)) {
            visit_paragraph(
                clang::dyn_cast<ParagraphComment>(*it), traits, description);
        }
    }

    if (!name.empty()) {
        if (!cmt.contains("param"))
            cmt["param"] = inja::json::array();

        inja::json param = inja::json::object();
        param["name"] = name;
        param["description"] = util::trim(description);
        cmt["param"].push_back(std::move(param));
    }
}

void clang_visitor::visit_tparam_command(
    const clang::comments::TParamCommandComment *command,
    const clang::comments::CommandTraits &traits, common::model::comment_t &cmt)
{
    using clang::comments::Comment;
    using clang::comments::ParagraphComment;
    using clang::comments::TextComment;

    std::string description;

    if (command == nullptr)
        return;

    const auto name = command->getParamNameAsWritten().str();

    for (const auto *it = command->child_begin(); it != command->child_end();
         ++it) {
        if ((*it)->getCommentKind() ==
            CLANG_UML_LLVM_COMMENT_KIND(ParagraphComment)) {
            visit_paragraph(
                clang::dyn_cast<ParagraphComment>(*it), traits, description);
        }
    }

    if (!name.empty()) {
        if (!cmt.contains("tparam"))
            cmt["tparam"] = inja::json::array();

        inja::json param = inja::json::object();
        param["name"] = name;
        param["description"] = util::trim(description);
        cmt["tparam"].push_back(std::move(param));
    }
}

void clang_visitor::visit_paragraph(
    const clang::comments::ParagraphComment *paragraph,
    const clang::comments::CommandTraits & /*traits*/, std::string &text)
{
    using clang::comments::Comment;
    using clang::comments::TextComment;

    if (paragraph == nullptr)
        return;

    for (const auto *text_it = paragraph->child_begin();
         text_it != paragraph->child_end(); ++text_it) {

        if ((*text_it)->getCommentKind() ==
                CLANG_UML_LLVM_COMMENT_KIND(TextComment) &&
            clang::dyn_cast<TextComment>(*text_it) != nullptr) {
            // Merge paragraph lines into a single string
            text += clang::dyn_cast<TextComment>(*text_it)->getText();
            text += "\n";
        }
    }
}

} // namespace clanguml::common::visitor::comment