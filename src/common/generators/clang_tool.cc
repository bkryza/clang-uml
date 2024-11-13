/**
 * @file src/common/generators/clang_tool.cc
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

#include "clang_tool.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Tooling/CompilationDatabase.h>

#include "util/util.h"

namespace clanguml::generators {

namespace {
void inject_resource_dir(
    CommandLineArguments &args, const char *argv_0, void *main_addr)
{
    using namespace std::string_literals;

    if (std::any_of(std::begin(args), std::end(args), [](const auto &arg) {
            return util::starts_with(arg, "-resource-dir"s);
        }))
        return;

    args = clang::tooling::getInsertArgumentAdjuster(("-resource-dir=" +
        clang::CompilerInvocation::GetResourcesPath(argv_0, main_addr))
                                                         .c_str())(args, "");
}
} // namespace

std::string to_string(const clanguml::generators::diagnostic &d)
{
    if (!d.location) {
        return fmt::format("[{}] {}", d.level, d.description);
    }

    return fmt::format("[{}] {}:{}: {}", d.level, d.location->file_relative(),
        d.location->line(), d.description);
}

clang_tool_exception::clang_tool_exception(
    common::model::diagram_t dt, std::string dn, std::vector<diagnostic> d)
    : error::diagram_generation_error{dt, dn, "Clang failed to parse sources."}
    , diagnostics{std::move(d)}
{
}

diagnostic_consumer::diagnostic_consumer(std::filesystem::path relative_to)
    : relative_to_{std::move(relative_to)}
{
}

void diagnostic_consumer::HandleDiagnostic(
    DiagnosticsEngine::Level diag_level, const Diagnostic &info)
{
    SmallVector<char> buf{};
    info.FormatDiagnostic(buf);

    diagnostic d;
    d.level = diag_level;
    d.description = std::string{buf.data(), buf.size()};

    if (info.hasSourceManager() && info.getLocation().isValid()) {
        d.location = clanguml::common::model::source_location();
        common::set_source_location(info.getSourceManager(), info.getLocation(),
            *d.location, {}, relative_to_);
    }

    if (diag_level == clang::DiagnosticsEngine::Level::Error ||
        diag_level == clang::DiagnosticsEngine::Level::Fatal) {
        failed = true;
    }

    diagnostics.emplace_back(std::move(d));
}

clang_tool::clang_tool(common::model::diagram_t diagram_type,
    std::string diagram_name,
    const clanguml::common::compilation_database &compilation_database,
    const std::vector<std::string> &source_paths,
    std::filesystem::path relative_to, bool quiet)
    : diagram_type_{diagram_type}
    , diagram_name_{std::move(diagram_name)}
    , compilations_{compilation_database}
    , source_paths_{source_paths}
    , quiet_{quiet}
    , pch_container_ops_{std::make_shared<PCHContainerOperations>()}
    , overlay_fs_{new llvm::vfs::OverlayFileSystem(
          llvm::vfs::getRealFileSystem())}
    , inmemory_fs_{new llvm::vfs::InMemoryFileSystem}
    , files_{new FileManager(FileSystemOptions(), overlay_fs_)}
    , diag_consumer_{std::make_unique<diagnostic_consumer>(relative_to)}
{
    overlay_fs_->pushOverlay(inmemory_fs_);

    append_arguments_adjuster(getClangStripOutputAdjuster());
    append_arguments_adjuster(getClangSyntaxOnlyAdjuster());
    append_arguments_adjuster(getClangStripDependencyFileAdjuster());
}

clang_tool::~clang_tool() = default;

void clang_tool::append_arguments_adjuster(ArgumentsAdjuster Adjuster)
{
    args_adjuster_ =
        combineAdjusters(std::move(args_adjuster_), std::move(Adjuster));
}

void clang_tool::run(ToolAction *Action)
{
    static int static_symbol;

    std::vector<std::string> absolute_tu_paths;
    absolute_tu_paths.reserve(source_paths_.size());
    for (const auto &source_path : source_paths_) {
        auto absolute_tu_path =
            clang::tooling::getAbsolutePath(*overlay_fs_, source_path);
        if (!absolute_tu_path) {
            if (!quiet_) {
                LOG_WARN("Skipping file {} in diagram {}. Could not resolve "
                         "absolute path for translation unit: {}",
                    source_path, diagram_name_,
                    llvm::toString(absolute_tu_path.takeError()));
            }
            continue;
        }
        absolute_tu_paths.emplace_back(std::move(*absolute_tu_path));
    }

    // Remember the working directory in case we need to restore it.
    std::string initial_workdir;
    if (auto current_workdir = overlay_fs_->getCurrentWorkingDirectory()) {
        initial_workdir = std::move(*current_workdir);
    }
    else {
        if (!quiet_)
            LOG_ERROR("Could not get current working directory when generating "
                      "diagram '{}': {}",
                diagram_name_, current_workdir.getError().message());
    }

    for (const auto &file : absolute_tu_paths) {
        if (!quiet_)
            LOG_INFO("Processing diagram '{}' translation unit: {}",
                diagram_name_, file);

        auto compile_commands_for_file = compilations_.getCompileCommands(file);

        if (compile_commands_for_file.empty()) {
            if (!quiet_)
                LOG_WARN(
                    "Skipping file {} for diagram '{}'. Compilation command "
                    "not found.",
                    file, diagram_name_);
            continue;
        }

        for (auto &compile_command : compile_commands_for_file) {
            if (overlay_fs_->setCurrentWorkingDirectory(
                    compile_command.Directory))
                llvm::report_fatal_error("Cannot chdir into \"" +
                    Twine(compile_command.Directory) + "\"!");

            // Now fill the in-memory VFS with the relative file mappings so it
            // will have the correct relative paths. We never remove mappings
            // but that should be fine.
            if (visited_working_directories_.insert(compile_command.Directory)
                    .second) {
                for (const auto &[file_name, file_content] :
                    memory_mapped_files_)
                    if (!llvm::sys::path::is_absolute(file_name))
                        inmemory_fs_->addFile(file_name, 0,
                            llvm::MemoryBuffer::getMemBuffer(file_content));
            }

            auto command_line = compile_command.CommandLine;
            if (args_adjuster_)
                command_line =
                    args_adjuster_(command_line, compile_command.Filename);

            assert(!command_line.empty());

            inject_resource_dir(command_line, "clang_tool", &static_symbol);

            ToolInvocation invocation(std::move(command_line), Action,
                files_.get(), pch_container_ops_);
            invocation.setDiagnosticConsumer(diag_consumer_.get());

            if (!invocation.run() || diag_consumer_->failed) {
                if (!initial_workdir.empty()) {
                    if (const auto ec = overlay_fs_->setCurrentWorkingDirectory(
                            initial_workdir);
                        ec)
                        if (!quiet_)
                            LOG_ERROR("Error when trying to restore working "
                                      "directory: {}",
                                ec.message());
                }

                if (diag_consumer_ && diag_consumer_->failed) {
                    throw clang_tool_exception(diagram_type_, diagram_name_,
                        diag_consumer_->diagnostics);
                }

                throw std::runtime_error(
                    fmt::format("Unknown error while processing {}", file));
            }
        }
    }

    if (!initial_workdir.empty()) {
        if (const auto ec =
                overlay_fs_->setCurrentWorkingDirectory(initial_workdir))
            if (!quiet_)
                LOG_ERROR("Error when trying to restore working dir: {}",
                    ec.message());
    }
}
} // namespace clanguml::generators

namespace clang {
std::string to_string(clang::DiagnosticsEngine::Level level)
{
    std::string level_str;
    switch (level) {
    case clang::DiagnosticsEngine::Ignored:
        level_str = "IGNORED";
        break;
    case clang::DiagnosticsEngine::Note:
        level_str = "NOTE";
        break;
    case clang::DiagnosticsEngine::Remark:
        level_str = "REMARK";
        break;
    case clang::DiagnosticsEngine::Warning:
        level_str = "WARNING";
        break;
    case clang::DiagnosticsEngine::Error:
        level_str = "ERROR";
        break;
    case clang::DiagnosticsEngine::Fatal:
        level_str = "FATAL";
        break;
    default:
        level_str = "UNKNOWN";
        break;
    }

    return level_str;
}
} // namespace clang
