/**
 * @file src/common/visitor/translation_unit_visitor.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

#include "comment/clang_visitor.h"
#include "comment/plain_visitor.h"

namespace clanguml::common::visitor {

translation_unit_visitor::translation_unit_visitor(
    clang::SourceManager &sm, const clanguml::config::diagram &config)
    : source_manager_{sm}
    , relative_to_path_{config.root_directory()}
{
    if (config.comment_parser() == config::comment_parser_t::plain) {
        comment_visitor_ =
            std::make_unique<comment::plain_visitor>(source_manager_);
    }
    else if (config.comment_parser() == config::comment_parser_t::clang) {
        comment_visitor_ =
            std::make_unique<comment::clang_visitor>(source_manager_);
    }
}

void translation_unit_visitor::set_tu_path(
    const std::string &translation_unit_path)
{
    translation_unit_path_ = relative(
        std::filesystem::path{translation_unit_path}, relative_to_path_);
    translation_unit_path_.make_preferred();
}

const std::filesystem::path &translation_unit_visitor::tu_path() const
{
    return translation_unit_path_;
}

clang::SourceManager &translation_unit_visitor::source_manager() const
{
    return source_manager_;
}

void translation_unit_visitor::process_comment(
    const clang::NamedDecl &decl, clanguml::common::model::decorated_element &e)
{
    assert(comment_visitor_.get() != nullptr);

    comment_visitor_->visit(decl, e);

    const auto *comment =
        decl.getASTContext().getRawCommentForDeclNoCache(&decl);

    process_comment(comment, decl.getASTContext().getDiagnostics(), e);
}

void translation_unit_visitor::process_comment(const clang::RawComment *comment,
    clang::DiagnosticsEngine &de, clanguml::common::model::decorated_element &e)
{
    if (comment != nullptr) {
        auto [it, inserted] = processed_comments_.emplace(comment);

        if (!inserted)
            return;

        // Process clang-uml decorators in the comments
        // TODO: Refactor to use standard block comments processable by clang
        //       comments
        e.add_decorators(
            decorators::parse(comment->getFormattedText(source_manager_, de)));
    }
}

void translation_unit_visitor::set_source_location(
    const clang::Decl &decl, clanguml::common::model::source_location &element)
{
    set_source_location(decl.getLocation(), element);
}

void translation_unit_visitor::set_source_location(
    const clang::Expr &expr, clanguml::common::model::source_location &element)
{
    set_source_location(expr.getBeginLoc(), element);
}

void translation_unit_visitor::set_source_location(
    const clang::Stmt &stmt, clanguml::common::model::source_location &element)
{
    set_source_location(stmt.getBeginLoc(), element);
}

void translation_unit_visitor::set_source_location(
    const clang::SourceLocation &location,
    clanguml::common::model::source_location &element)
{
    namespace fs = std::filesystem;

    std::string file;
    unsigned line{};
    unsigned column{};

    if (location.isValid()) {
        file = source_manager_.getFilename(location).str();
        line = source_manager_.getSpellingLineNumber(location);
        column = source_manager_.getSpellingColumnNumber(location);

        if (file.empty()) {
            // Why do I have to do this?
            parse_source_location(
                location.printToString(source_manager()), file, line, column);
        }
    }
    else {
        auto success = parse_source_location(
            location.printToString(source_manager()), file, line, column);
        if (!success) {
            LOG_DBG("Failed to extract source location for element from {}",
                location.printToString(source_manager_));
            return;
        }
    }

    // ensure the path is absolute
    fs::path file_path{file};
    if (!file_path.is_absolute()) {
        file_path = fs::absolute(file_path);
    }

    file_path = fs::canonical(file_path);

    file = file_path.string();

    element.set_file(file);

    if (util::is_relative_to(file_path, relative_to_path_)) {
        element.set_file_relative(util::path_to_url(
            fs::relative(element.file(), relative_to_path_).string()));
    }
    else {
        element.set_file_relative("");
    }

    element.set_translation_unit(tu_path().string());
    element.set_line(line);
    element.set_column(column);
    element.set_location_id(location.getHashValue());
}

} // namespace clanguml::common::visitor