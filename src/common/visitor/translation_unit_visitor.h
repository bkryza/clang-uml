/**
 * @file src/common/visitor/translation_unit_visitor.h
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

#include "comment/clang_visitor.h"
#include "comment/comment_visitor.h"
#include "comment/plain_visitor.h"
#include "common/clang_utils.h"
#include "common/model/namespace.h"
#include "common/model/source_file.h"
#include "common/model/source_location.h"
#include "common/model/template_element.h"
#include "common/visitor/ast_id_mapper.h"
#include "config/config.h"

#include <clang/AST/Comment.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RawCommentList.h>
#include <clang/Basic/Module.h>
#include <clang/Basic/SourceManager.h>

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::common::visitor {

using found_relationships_t =
    std::vector<std::pair<eid_t, common::model::relationship_t>>;

/**
 * @brief Diagram translation unit visitor base class
 *
 * This class provides common interface for diagram translation unit
 * visitors.
 */
template <typename ConfigT, typename DiagramT> class translation_unit_visitor {
public:
    using config_t = ConfigT;
    using diagram_t = DiagramT;

    /**
     * @brief Constructor
     *
     * @param sm Reference to @ref clang::SourceManager instance
     * @param config Reference to @ref clanguml::config::diagram configuration
     *        instance
     */
    explicit translation_unit_visitor(
        clang::SourceManager &sm, DiagramT &diagram, const ConfigT &config)
        : diagram_{diagram}
        , config_{config}
        , source_manager_{sm}
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

    virtual ~translation_unit_visitor() = default;

    void set_tu_path(const std::string &translation_unit_path)
    {
        translation_unit_path_ = relative(
            std::filesystem::path{translation_unit_path}, relative_to_path_);
        translation_unit_path_.make_preferred();
    }

    /**
     * @brief Return relative path to current translation unit
     * @return Current translation unit path
     */
    const std::filesystem::path &tu_path() const
    {
        return translation_unit_path_;
    }

    /**
     * @brief Get reference to Clang AST to clang-uml id mapper
     *
     * @return Reference to Clang AST to clang-uml id mapper
     */
    common::visitor::ast_id_mapper &id_mapper() const { return id_mapper_; }

    /**
     * @brief Get clang::SourceManager
     * @return Reference to @ref clang::SourceManager used by this translation
     *         unit visitor
     */
    clang::SourceManager &source_manager() const { return source_manager_; }

    /**
     * @brief Set source location in diagram element
     *
     * @param decl Reference to @ref clang::Decl
     * @param element Reference to element to be updated
     */
    void set_source_location(const clang::Decl &decl,
        clanguml::common::model::source_location &element)
    {
        set_source_location(decl.getLocation(), element);
    }

    /**
     * @brief Set source location in diagram element
     *
     * @param expr Reference to @ref clang::Expr
     * @param element Reference to element to be updated
     */
    void set_source_location(const clang::Expr &expr,
        clanguml::common::model::source_location &element)
    {
        set_source_location(expr.getBeginLoc(), element);
    }

    void set_source_location(const clang::Stmt &stmt,
        clanguml::common::model::source_location &element)
    {
        set_source_location(stmt.getBeginLoc(), element);
    }

    void set_qualified_name(
        const clang::NamedDecl &decl, clanguml::common::model::element &element)
    {
        common::model::namespace_ ns{decl.getQualifiedNameAsString()};
        element.set_name(ns.name());
        ns.pop_back();
        element.set_namespace(ns);
    }

    /**
     * @brief Set source location in diagram element
     *
     * @param location Reference to @ref clang::SourceLocation
     * @param element Reference to element to be updated
     */
    void set_source_location(const clang::SourceLocation &location,
        clanguml::common::model::source_location &element)
    {
        common::set_source_location(
            source_manager(), location, element, tu_path(), relative_to_path_);
    }

    void set_owning_module(
        const clang::Decl &decl, clanguml::common::model::element &element)
    {
        if (const clang::Module *module = decl.getOwningModule();
            module != nullptr) {
            std::string module_name = module->Name;
            bool is_private{false};
#if LLVM_VERSION_MAJOR < 15
            is_private = module->Kind ==
                clang::Module::ModuleKind::PrivateModuleFragment;
#else
            is_private = module->isPrivateModule();
#endif
            if (is_private) {
                // Clang just maps private modules names to "<private>"
                module_name = module->getTopLevelModule()->Name;
            }
            element.set_module(module_name);
            element.set_module_private(is_private);
        }
    }

    virtual void add_diagram_element(
        std::unique_ptr<common::model::template_element> element)
    {
    }

    /**
     * @brief Process comment directives in comment attached to a declaration
     *
     * @param decl Reference to @ref clang::NamedDecl
     * @param element Reference to element to be updated
     */
    void process_comment(const clang::NamedDecl &decl,
        clanguml::common::model::decorated_element &e)
    {
        assert(comment_visitor_.get() != nullptr);

        comment_visitor_->visit(decl, e);

        auto *comment = decl.getASTContext().getRawCommentForDeclNoCache(&decl);

        process_comment(comment, decl.getASTContext().getDiagnostics(), e);
    }

    /**
     * @brief Process comment directives in raw comment
     *
     * @param comment clang::RawComment pointer
     * @param de Reference to clang::DiagnosticsEngine
     * @param element Reference to element to be updated
     * @return Comment with uml directives stripped from it
     */
    [[maybe_unused]] std::string process_comment(
        const clang::RawComment *comment, clang::DiagnosticsEngine &de,
        clanguml::common::model::decorated_element &e)
    {
        if (comment == nullptr)
            return {};

        auto [it, inserted] = processed_comments().emplace(comment);

        if (!inserted)
            return {};

        // Process clang-uml decorators in the comments
        // TODO: Refactor to use standard block comments processable by
        //       clang comments
        const auto &[decorators, stripped_comment] =
            decorators::parse(comment->getFormattedText(source_manager(), de));

        e.add_decorators(decorators);

        return stripped_comment;
    }

    bool skip_system_header_decl(const clang::NamedDecl *decl) const
    {
        return !config().include_system_headers() &&
            source_manager().isInSystemHeader(
                decl->getSourceRange().getBegin());
    }

    /**
     * @brief Check if the diagram should include a declaration.
     *
     * @param decl Clang declaration.
     * @return True, if the entity should be included in the diagram.
     */
    bool should_include(const clang::NamedDecl *decl) const
    {
        if (decl == nullptr)
            return false;

        if (skip_system_header_decl(decl))
            return false;

        if (config().filter_mode() == config::filter_mode_t::advanced)
            return true;

        auto should_include_namespace = diagram().should_include(
            common::model::namespace_{decl->getQualifiedNameAsString()});

        const auto decl_file =
            decl->getLocation().printToString(source_manager());

        const auto should_include_decl_file = diagram().should_include(
            common::model::source_file{get_file_path(decl_file)});

        return should_include_namespace && should_include_decl_file;
    }

    /**
     * @brief Get diagram model reference
     *
     * @return Reference to diagram model created by the visitor
     */
    DiagramT &diagram() { return diagram_; }

    /**
     * @brief Get diagram model reference
     *
     * @return Reference to diagram model created by the visitor
     */
    const DiagramT &diagram() const { return diagram_; }

    /**
     * @brief Get diagram config instance
     *
     * @return Reference to config instance
     */
    const ConfigT &config() const { return config_; }

protected:
    std::set<const clang::RawComment *> &processed_comments()
    {
        return processed_comments_;
    }

    std::string get_file_path(const std::string &file_location) const
    {
        std::string file_path;
        unsigned line{};
        unsigned column{};

        if (common::parse_source_location(
                file_location, file_path, line, column))
            return file_path;

        return file_location;
    }

private:
    // Reference to the output diagram model
    DiagramT &diagram_;

    // Reference to class diagram config
    const ConfigT &config_;

    clang::SourceManager &source_manager_;

    std::unique_ptr<comment::comment_visitor> comment_visitor_;

    std::filesystem::path relative_to_path_;

    std::filesystem::path translation_unit_path_;

    std::set<const clang::RawComment *> processed_comments_;

    mutable common::visitor::ast_id_mapper id_mapper_;
};
} // namespace clanguml::common::visitor
