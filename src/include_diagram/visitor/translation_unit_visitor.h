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
#include <clang/Lex/PPCallbacks.h>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml::include_diagram::visitor {

class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor> {
public:
    // This is an internal class for convenience to be able to access the
    // include_visitor type from translation_unit_visitor type
    class include_visitor : public clang::PPCallbacks {
    public:
        include_visitor(clang::SourceManager &sm,
            clanguml::include_diagram::model::diagram &diagram,
            const clanguml::config::include_diagram &config);

        void InclusionDirective(clang::SourceLocation hash_loc,
            const clang::Token &include_tok, clang::StringRef file_name,
            bool is_angled, clang::CharSourceRange filename_range,
            const clang::FileEntry *file, clang::StringRef search_path,
            clang::StringRef relative_path, const clang::Module *imported,
            clang::SrcMgr::CharacteristicKind file_type) override;

        std::optional<common::id_t> process_internal_header(const std::filesystem::path &include_path,
            bool is_system, const common::id_t current_file_id);

        std::optional<common::id_t> process_external_system_header(
            const std::filesystem::path &include_path,
            const common::id_t current_file_id);

        std::optional<common::id_t> process_source_file(const std::filesystem::path &file);

        clanguml::include_diagram::model::diagram &diagram()
        {
            return diagram_;
        }

        const clanguml::config::include_diagram &config() const
        {
            return config_;
        }

    private:
        clang::SourceManager &source_manager_;

        // Reference to the output diagram model
        clanguml::include_diagram::model::diagram &diagram_;

        // Reference to class diagram config
        const clanguml::config::include_diagram &config_;
    };

    translation_unit_visitor(clang::SourceManager &sm,
        clanguml::include_diagram::model::diagram &diagram,
        const clanguml::config::include_diagram &config);

    clanguml::include_diagram::model::diagram &diagram() { return diagram_; }

    const clanguml::config::include_diagram &config() const { return config_; }

    void finalize() { }

private:
    clang::SourceManager &source_manager_;

    // Reference to the output diagram model
    clanguml::include_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::include_diagram &config_;
};
}
