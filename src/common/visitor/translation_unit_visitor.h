/**
 * @file src/common/visitor/translation_unit_visitor.h
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
#pragma once

#include "comment/comment_visitor.h"
#include "common/model/element.h"
#include "common/model/source_location.h"
#include "config/config.h"

#include <clang/AST/Comment.h>
#include <clang/AST/RawCommentList.h>
#include <clang/Basic/SourceManager.h>

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::common::visitor {

using found_relationships_t = std::vector<
    std::pair<clanguml::common::id_t, common::model::relationship_t>>;

/**
 * @brief Diagram translation unit visitor base class
 *
 * This class provides common interface for diagram translation unit
 * visitors.
 */
class translation_unit_visitor {
public:
    /**
     * @brief Constructor
     *
     * @param sm Reference to @ref clang::SourceManager instance
     * @param config Reference to @ref clanguml::config::diagram configuration
     *        instance
     */
    explicit translation_unit_visitor(
        clang::SourceManager &sm, const clanguml::config::diagram &config);

    virtual ~translation_unit_visitor() = default;

    void set_tu_path(const std::string &translation_unit_path);

    /**
     * @brief Return relative path to current translation unit
     * @return Current translation unit path
     */
    const std::filesystem::path &tu_path() const;

    /**
     * @brief Get clang::SourceManager
     * @return Reference to @ref clang::SourceManager used by this translation
     *         unit visitor
     */
    clang::SourceManager &source_manager() const;

    /**
     * @brief Set source location in diagram element
     *
     * @param decl Reference to @ref clang::Decl
     * @param element Reference to element to be updated
     */
    void set_source_location(const clang::Decl &decl,
        clanguml::common::model::source_location &element);

    /**
     * @brief Set source location in diagram element
     *
     * @param expr Reference to @ref clang::Expr
     * @param element Reference to element to be updated
     */
    void set_source_location(const clang::Expr &expr,
        clanguml::common::model::source_location &element);

    void set_source_location(const clang::Stmt &stmt,
        clanguml::common::model::source_location &element);

    /**
     * @brief Set source location in diagram element
     *
     * @param location Reference to @ref clang::SourceLocation
     * @param element Reference to element to be updated
     */
    void set_source_location(const clang::SourceLocation &location,
        clanguml::common::model::source_location &element);

    void set_owning_module(
        const clang::Decl &decl, clanguml::common::model::element &element);

protected:
    /**
     * @brief Process comment directives in comment attached to a declaration
     *
     * @param decl Reference to @ref clang::NamedDecl
     * @param element Reference to element to be updated
     */
    void process_comment(const clang::NamedDecl &decl,
        clanguml::common::model::decorated_element &e);

    /**
     * @brief Process comment directives in raw comment
     *
     * @param comment clang::RawComment pointer
     * @param de Reference to clang::DiagnosticsEngine
     * @param element Reference to element to be updated
     */
    void process_comment(const clang::RawComment *comment,
        clang::DiagnosticsEngine &de,
        clanguml::common::model::decorated_element &e);

private:
    clang::SourceManager &source_manager_;

    std::unique_ptr<comment::comment_visitor> comment_visitor_;

    std::filesystem::path relative_to_path_;

    std::filesystem::path translation_unit_path_;

    std::set<const clang::RawComment *> processed_comments_;
};
} // namespace clanguml::common::visitor
