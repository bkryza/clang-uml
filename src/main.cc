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

#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "cli/cli_handler.h"
#include "config/config.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "util/util.h"
#include "version.h"

#ifdef ENABLE_BACKWARD_CPP
#define BACKWARD_HAS_DW 1
#define BACKWARD_HAS_LIBUNWIND 1
#include <backward-cpp/backward.hpp>
#endif

#include <cli11/CLI11.hpp>
#include <spdlog/spdlog.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <util/thread_pool_executor.h>

#ifdef ENABLE_BACKWARD_CPP
namespace backward {
backward::SignalHandling sh; // NOLINT
} // namespace backward
#endif

using namespace clanguml;

/**
 * Generate specific diagram identified by name
 *
 * @param od Diagram output directory
 * @param name Name of the diagram as specified in the config file
 * @param diagram Diagram model instance
 * @param db Compilation database
 * @param translation_units List of translation units to be used for this
 *        diagram
 * @param verbose Logging level
 */
void generate_diagram(const std::string &od, const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const clang::tooling::CompilationDatabase &db,
    const std::vector<std::string> &translation_units, bool verbose);

/**
 * Find translation units for diagrams.
 *
 *  For each diagram to be generated, this function selects translation units
 *  to be used for this diagram. The files are selected as an intersection
 *  between all translation units found in the compilation database and the
 *  `glob` patterns specified for each diagram in the configuration file.
 *
 *  @param diagram_names List of diagram names to be generated
 *  @param config Configuration instance
 *  @param compilation_database_files All translation units in compilation
 *  database
 *  @param translation_units_map The output map containing translation
 *                  units for each diagram by name
 */
void find_translation_units_for_diagrams(
    const std::vector<std::string> &diagram_names,
    clanguml::config::config &config,
    const std::vector<std::string> &compilation_database_files,
    std::map<std::string, std::vector<std::string>> &translation_units_map);

/**
 *  Generate diagrams.
 *
 *  This function generates all diagrams specified in the configuration file
 *  and in the command line.
 *
 *  @param diagram_names List of diagram names to be generated
 *  @param config Configuration instance
 *  @param od Output directory where diagrams should be written
 *  @param db Compilation database instance
 *  @param verbose Verbosity level
 *  @param thread_count Number of diagrams to be generated in parallel
 *  @param translation_units_map List of translation units to be used for each
 *         diagram
 */
void generate_diagrams(const std::vector<std::string> &diagram_names,
    clanguml::config::config &config, const std::string &od,
    const std::unique_ptr<clang::tooling::CompilationDatabase> &db, int verbose,
    unsigned int thread_count,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map);

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
    find_translation_units_for_diagrams(cli.diagram_names, cli.config,
        compilation_database_files, translation_units_map);

    generate_diagrams(cli.diagram_names, cli.config,
        cli.effective_output_directory, db, cli.verbose, cli.thread_count,
        translation_units_map);

    return 0;
}

void generate_diagram(const std::string &od, const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const clang::tooling::CompilationDatabase &db,
    const std::vector<std::string> &translation_units, bool verbose)
{
    using clanguml::common::model::diagram_t;
    using clanguml::config::class_diagram;
    using clanguml::config::include_diagram;
    using clanguml::config::package_diagram;
    using clanguml::config::sequence_diagram;

    auto path = std::filesystem::path{od} / fmt::format("{}.puml", name);
    std::ofstream ofs;
    ofs.open(path, std::ofstream::out | std::ofstream::trunc);

    if (diagram->type() == diagram_t::kClass) {
        using diagram_config = class_diagram;
        using diagram_model = clanguml::class_diagram::model::diagram;
        using diagram_visitor =
            clanguml::class_diagram::visitor::translation_unit_visitor;

        auto model =
            clanguml::common::generators::plantuml::generate<diagram_model,
                diagram_config, diagram_visitor>(db, diagram->name,
                dynamic_cast<diagram_config &>(*diagram), translation_units,
                verbose);

        ofs << clanguml::class_diagram::generators::plantuml::generator(
            dynamic_cast<diagram_config &>(*diagram), *model);
    }
    else if (diagram->type() == diagram_t::kSequence) {
        using diagram_config = sequence_diagram;
        using diagram_model = clanguml::sequence_diagram::model::diagram;
        using diagram_visitor =
            clanguml::sequence_diagram::visitor::translation_unit_visitor;

        auto model =
            clanguml::common::generators::plantuml::generate<diagram_model,
                diagram_config, diagram_visitor>(db, diagram->name,
                dynamic_cast<diagram_config &>(*diagram), translation_units,
                verbose);

        ofs << clanguml::sequence_diagram::generators::plantuml::generator(
            dynamic_cast<clanguml::config::sequence_diagram &>(*diagram),
            *model);
    }
    else if (diagram->type() == diagram_t::kPackage) {
        using diagram_config = package_diagram;
        using diagram_model = clanguml::package_diagram::model::diagram;
        using diagram_visitor =
            clanguml::package_diagram::visitor::translation_unit_visitor;

        auto model =
            clanguml::common::generators::plantuml::generate<diagram_model,
                diagram_config, diagram_visitor>(db, diagram->name,
                dynamic_cast<diagram_config &>(*diagram), translation_units,
                verbose);

        ofs << clanguml::package_diagram::generators::plantuml::generator(
            dynamic_cast<diagram_config &>(*diagram), *model);
    }
    else if (diagram->type() == diagram_t::kInclude) {
        using diagram_config = include_diagram;
        using diagram_model = clanguml::include_diagram::model::diagram;
        using diagram_visitor =
            clanguml::include_diagram::visitor::translation_unit_visitor;

        auto model =
            clanguml::common::generators::plantuml::generate<diagram_model,
                diagram_config, diagram_visitor>(db, diagram->name,
                dynamic_cast<diagram_config &>(*diagram), translation_units,
                verbose);

        ofs << clanguml::include_diagram::generators::plantuml::generator(
            dynamic_cast<diagram_config &>(*diagram), *model);
    }

    LOG_INFO("Written {} diagram to {}", name, path.string());

    ofs.close();
}

void generate_diagrams(const std::vector<std::string> &diagram_names,
    clanguml::config::config &config, const std::string &od,
    const std::unique_ptr<clang::tooling::CompilationDatabase> &db,
    const int verbose, const unsigned int thread_count,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map)
{
    util::thread_pool_executor generator_executor{thread_count};
    std::vector<std::future<void>> futs;

    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command line,
        // and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        const auto &valid_translation_units = translation_units_map.at(name);

        if (valid_translation_units.empty()) {
            LOG_ERROR(
                "Diagram {} generation failed: no translation units found",
                name);
            continue;
        }

        futs.emplace_back(generator_executor.add(
            [&od, &name = name, &diagram = diagram, db = std::ref(*db),
                translation_units = valid_translation_units, verbose]() {
                try {
                    generate_diagram(
                        od, name, diagram, db, translation_units, verbose != 0);
                }
                catch (std::runtime_error &e) {
                    LOG_ERROR(e.what());
                }
            }));
    }

    for (auto &fut : futs) {
        fut.get();
    }
}

void find_translation_units_for_diagrams(
    const std::vector<std::string> &diagram_names,
    clanguml::config::config &config,
    const std::vector<std::string> &compilation_database_files,
    std::map<std::string, std::vector<std::string>> &translation_units_map)
{
    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command line,
        // and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        // If glob is not defined use all translation units from the
        // compilation database
        if (!diagram->glob.has_value) {
            translation_units_map[name] = compilation_database_files;
        }
        // Otherwise, get all translation units matching the glob from diagram
        // configuration
        else {
            const std::vector<std::string> translation_units =
                diagram->get_translation_units();

            std::vector<std::string> valid_translation_units{};
            std::copy_if(compilation_database_files.begin(),
                compilation_database_files.end(),
                std::back_inserter(valid_translation_units),
                [&translation_units](const auto &tu) {
                    return util::contains(translation_units, tu);
                });

            translation_units_map[name] = std::move(valid_translation_units);
        }
    }
}
