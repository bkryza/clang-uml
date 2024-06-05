/**
 * @file src/include_diagram/visitor/translation_unit_visitor.h
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

#include "common/model/enums.h"
#include "common/model/package.h"
#include "common/visitor/translation_unit_visitor.h"
#include "config/config.h"
#include "include_diagram/model/diagram.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/PPCallbacks.h>

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace clanguml::include_diagram::visitor {

using clanguml::common::eid_t;

using visitor_specialization_t =
    common::visitor::translation_unit_visitor<clanguml::config::include_diagram,
        clanguml::include_diagram::model::diagram>;

/**
 * @brief Include diagram translation unit visitor wrapper
 *
 * This class implements the @link clang::RecursiveASTVisitor interface,
 * for compatibility with other diagram visitors. However, for include
 * diagrams this class does not inherit from
 * @ref common::visitor::translation_unit_visitor, instead it contains an
 * inner class @ref include_visitor, which implements `clang::PPCallbacks`
 * interface to handle inclusion directives.
 */
class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor> {
public:
    /**
     * This is an internal class for convenience to be able to access the
     * include_visitor type from translation_unit_visitor type
     */
    class include_visitor : public clang::PPCallbacks,
                            public visitor_specialization_t {
    public:
        /**
         * @brief Constructor.
         *
         * @param sm Reference to current tu source manager.
         * @param diagram Reference to the include diagram model.
         * @param config Reference to the diagram configuration.
         */
        include_visitor(clang::SourceManager &sm,
            clanguml::include_diagram::model::diagram &diagram,
            const clanguml::config::include_diagram &config);

        ~include_visitor() override = default;

#if LLVM_VERSION_MAJOR >= 16
        void InclusionDirective(clang::SourceLocation HashLoc,
            const clang::Token &IncludeTok, clang::StringRef FileName,
            bool IsAngled, clang::CharSourceRange FilenameRange,
            clang::OptionalFileEntryRef File, clang::StringRef SearchPath,
            clang::StringRef RelativePath, const clang::Module *Imported,
            clang::SrcMgr::CharacteristicKind FileType) override;
#elif LLVM_VERSION_MAJOR > 14
        void InclusionDirective(clang::SourceLocation hash_loc,
            const clang::Token &include_tok, clang::StringRef file_name,
            bool is_angled, clang::CharSourceRange filename_range,
            clang::Optional<clang::FileEntryRef> file,
            clang::StringRef search_path, clang::StringRef relative_path,
            const clang::Module *imported,
            clang::SrcMgr::CharacteristicKind file_type) override;
#else
        void InclusionDirective(clang::SourceLocation hash_loc,
            const clang::Token &include_tok, clang::StringRef file_name,
            bool is_angled, clang::CharSourceRange filename_range,
            const clang::FileEntry *file, clang::StringRef search_path,
            clang::StringRef relative_path, const clang::Module *imported,
            clang::SrcMgr::CharacteristicKind file_type) override;
#endif

        /**
         * @brief Handle internal header include directive
         *
         * @param include_path Include path
         * @param is_system True, if the path points to a system path
         * @param current_file_id File id
         */
        void process_internal_header(const std::filesystem::path &include_path,
            bool is_system, eid_t current_file_id);

        /**
         * @brief Handle system header include directive
         *
         * @param include_path Include path
         * @param current_file_id File id
         */
        void process_external_system_header(
            const std::filesystem::path &include_path, eid_t current_file_id);

        /**
         * @brief Handle a source file
         *
         * This method allows to process path of the currently visited
         * source file.
         *
         * @param file Absolute path to a source file
         * @return Diagram element id, in case the file was added to the diagram
         */
        std::optional<eid_t> process_source_file(
            const std::filesystem::path &file);
    };

    /**
     * @brief Constructor
     *
     * @param sm Reference to the source manager for current tu
     * @param diagram Reference to the include diagram model
     * @param config Reference to the diagram configuration
     */
    translation_unit_visitor(clang::SourceManager &sm,
        clanguml::include_diagram::model::diagram &diagram,
        const clanguml::config::include_diagram &config);

    /**
     * @brief Get reference to the include diagram model
     *
     * @return Reference to the include diagram model
     */
    clanguml::include_diagram::model::diagram &diagram();

    /**
     * @brief Get reference to the diagram configuration
     *
     * @return Reference to the diagram configuration
     */
    const clanguml::config::include_diagram &config() const;

    /**
     * @brief Run any finalization after traversal is complete
     */
    void finalize();

private:
    // Reference to the output diagram model
    clanguml::include_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::include_diagram &config_;
};
} // namespace clanguml::include_diagram::visitor
