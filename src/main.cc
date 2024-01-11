/**
 * @file src/main.cc
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

#include "cli/cli_handler.h"
#include "common/compilation_database.h"
#include "common/generators/generators.h"
#include "util/query_driver_output_extractor.h"
#include "util/util.h"

#ifdef ENABLE_BACKWARD_CPP
#define BACKWARD_HAS_DW 1
#define BACKWARD_HAS_LIBUNWIND 1
#include <backward-cpp/backward.hpp>
#endif

#include <cli11/CLI11.hpp>
#include <spdlog/spdlog.h>

#include <cstring>

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

#if !defined(NDEBUG)
    // Catch invalid logger message formats, e.g. missing arguments
    spdlog::set_error_handler([](const std::string & /*msg*/) {
        assert(0 == 1); // NOLINT
    });
#endif

    try {
        const auto db =
            common::compilation_database::auto_detect_from_directory(
                cli.config);

        const auto compilation_database_files = db->getAllFiles();

        std::map<std::string /* diagram name */,
            std::vector<std::string> /* translation units */>
            translation_units_map;

        // We have to generate the translation units list for each diagram
        // before scheduling tasks, because std::filesystem::current_path
        // cannot be trusted with multiple threads
        common::generators::find_translation_units_for_diagrams(
            cli.diagram_names, cli.config, compilation_database_files,
            translation_units_map);

        common::generators::generate_diagrams(cli.diagram_names, cli.config, db,
            cli.get_runtime_config(), translation_units_map);
    }
    catch (error::compilation_database_error &e) {
        LOG_ERROR("Failed to load compilation database from {} due to: {}",
            cli.config.compilation_database_dir(), e.what());
        return 1;
    }
    catch (error::query_driver_no_paths &e) {
        LOG_ERROR("Querying provided compiler driver {} did not provide any "
                  "paths, please make sure the path is correct and that your "
                  "compiler is GCC-compatible: {}",
            cli.config.query_driver(), e.what());
        return 1;
    }

    return 0;
}
