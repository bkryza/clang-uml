/**
 * src/main.cc
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

#include "cli/cli_handler.h"
#include "common/generators/generators.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "util/util.h"

#ifdef ENABLE_BACKWARD_CPP
#define BACKWARD_HAS_DW 1
#define BACKWARD_HAS_LIBUNWIND 1
#include <backward-cpp/backward.hpp>
#endif

#include <cli11/CLI11.hpp>
#include <spdlog/spdlog.h>

#include <cstring>
#include <iostream>
#include <util/thread_pool_executor.h>

#ifdef ENABLE_BACKWARD_CPP
namespace backward {
backward::SignalHandling sh; // NOLINT
} // namespace backward
#endif

using namespace clanguml;

int main(int argc, const char *argv[])
{
    cli::cli_handler cli;
    auto res = cli.handle_options(argc, argv);

    if (res == cli::cli_flow_t::kExit)
        return 0;

    if (res == cli::cli_flow_t::kError)
        return 1;

    std::string err{};
    auto db = clang::tooling::CompilationDatabase::autoDetectFromDirectory(
        cli.config.compilation_database_dir(), err);

    if (!err.empty()) {
        LOG_ERROR("Failed to load compilation database from {}",
            cli.config.compilation_database_dir());
        return 1;
    }

    const auto compilation_database_files = db->getAllFiles();

    std::map<std::string /* diagram name */,
        std::vector<std::string> /*translation units*/>
        translation_units_map;

    // We have to generate the translation units list for each diagram before
    // scheduling tasks, because std::filesystem::current_path cannot be trusted
    // with multiple threads
    clanguml::common::generators::find_translation_units_for_diagrams(
        cli.diagram_names, cli.config, compilation_database_files,
        translation_units_map);

    clanguml::common::generators::generate_diagrams(cli.diagram_names,
        cli.config, cli.effective_output_directory, db, cli.verbose,
        cli.thread_count, cli.generators, translation_units_map);

    return 0;
}
