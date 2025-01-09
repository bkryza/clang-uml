/**
 * @file src/common/generators/clang_tool.h
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include <clang/Tooling/Tooling.h>

#include "common/clang_utils.h"
#include "common/compilation_database.h"
#include "common/model/source_location.h"

namespace clanguml::generators {

using namespace clang;
using namespace clang::tooling;

struct diagnostic {
    clang::DiagnosticsEngine::Level level{
        clang::DiagnosticsEngine::Level::Ignored};
    std::string description;
    std::optional<clanguml::common::model::source_location> location;
};

std::string to_string(const diagnostic &d);

void to_json(nlohmann::json &j, const diagnostic &a);

class clang_tool_exception : public error::diagram_generation_error {
public:
    clang_tool_exception(common::model::diagram_t dt, std::string dn,
        std::vector<diagnostic> d,
        std::string description = "Clang failed to parse sources.");

    std::vector<diagnostic> diagnostics;
};

class diagnostic_consumer : public clang::DiagnosticConsumer {
public:
    diagnostic_consumer(std::filesystem::path relative_to);

    void HandleDiagnostic(
        DiagnosticsEngine::Level diag_level, const Diagnostic &info) override;

    /** If true, at least one of the diagnostics represents an error */
    bool failed{false};

    /** List of all diagnostics collected for a given TU */
    std::vector<diagnostic> diagnostics;

private:
    std::filesystem::path relative_to_;
};

/**
 * @brief Custom ClangTool implementation to enable better error handling
 */
class clang_tool {
public:
    clang_tool(common::model::diagram_t diagram_type, std::string diagram_name,
        const clanguml::common::compilation_database &compilations,
        const std::vector<std::string> &source_paths,
        std::filesystem::path relative_to, bool quiet);

    ~clang_tool();

    void append_arguments_adjuster(clang::tooling::ArgumentsAdjuster Adjuster);

    void run(ToolAction *Action);

private:
    const common::model::diagram_t diagram_type_;
    const std::string diagram_name_;
    const clanguml::common::compilation_database &compilations_;
    std::vector<std::string> source_paths_;
    bool quiet_;

    std::shared_ptr<PCHContainerOperations> pch_container_ops_;

    llvm::IntrusiveRefCntPtr<llvm::vfs::OverlayFileSystem> overlay_fs_;
    llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> inmemory_fs_;
    llvm::IntrusiveRefCntPtr<FileManager> files_;

    std::vector<
        std::pair<StringRef /* file_name */, StringRef /* file_contents */>>
        memory_mapped_files_;

    llvm::StringSet<> visited_working_directories_;

    ArgumentsAdjuster args_adjuster_;

    std::unique_ptr<diagnostic_consumer> diag_consumer_;
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diag_opts_;
};
} // namespace clanguml::generators

namespace clang {
std::string to_string(clang::DiagnosticsEngine::Level level);
} // namespace clang

MAKE_TO_STRING_FORMATTER(clang::DiagnosticsEngine::Level)
MAKE_TO_STRING_FORMATTER(clanguml::generators::diagnostic)
