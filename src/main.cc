/**
 * src/main.cc
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

#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "config/config.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "util/util.h"
#include "version.h"

#include <clang/Basic/Version.h>
#include <clang/Config/config.h>
#include <cli11/CLI11.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string.h>
#include <util/thread_pool_executor.h>

using namespace clanguml;
using config::config;

void print_version();

void print_diagrams_list(const clanguml::config::config &cfg);

bool check_output_directory(const std::string &dir);

void generate_diagram(const std::string &od, const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const clang::tooling::CompilationDatabase &db,
    const std::vector<std::string> &translation_units, bool verbose);

int main(int argc, const char *argv[])
{
    CLI::App app{"Clang-based PlantUML diagram generator for C++"};

    std::string config_path{".clang-uml"};
    std::string compilation_database_dir{'.'};
    std::vector<std::string> diagram_names{};
    std::optional<std::string> output_directory;
    unsigned int thread_count{0};
    bool show_version{false};
    int verbose{0};
    bool list_diagrams{false};

    app.add_option(
        "-c,--config", config_path, "Location of configuration file");
    app.add_option("-d,--compile-database", compilation_database_dir,
        "Location of compilation database directory");
    app.add_option("-n,--diagram-name", diagram_names,
        "List of diagram names to generate");
    app.add_option("-o,--output-directory", output_directory,
        "Override output directory specified in config file");
    app.add_option("-t,--thread-count", thread_count,
        "Thread pool size (0 = hardware concurrency)");
    app.add_flag("-V,--version", show_version, "Print version and exit");
    app.add_flag("-v,--verbose", verbose, "Verbose logging");
    app.add_flag("-l,--list-diagrams", list_diagrams,
        "Print list of diagrams defined in the config file");

    CLI11_PARSE(app, argc, argv);

    if (show_version) {
        print_version();
        return 0;
    }

    clanguml::util::setup_logging(verbose);

    clanguml::config::config config;
    try {
        config = clanguml::config::load(config_path);
    }
    catch (std::runtime_error &e) {
        LOG_ERROR(e.what());
        return 1;
    }

    if (list_diagrams) {
        print_diagrams_list(config);
        return 0;
    }

    LOG_INFO("Loaded clang-uml config from {}", config_path);

    LOG_INFO("Loading compilation database from {} directory",
        config.compilation_database_dir());

    auto od = config.output_directory();
    if (output_directory)
        od = output_directory.value();

    if (!check_output_directory(od))
        return 1;

    util::thread_pool_executor generator_executor{thread_count};
    std::vector<std::future<void>> futs;

    std::string err{};
    auto db = clang::tooling::CompilationDatabase::autoDetectFromDirectory(
        config.compilation_database_dir(), err);

    if (!err.empty()) {
        LOG_ERROR("Failed to load compilation database from {}",
            config.compilation_database_dir());
        return 1;
    }

    const auto compilation_database_files = db->getAllFiles();

    const auto current_directory = std::filesystem::current_path();

    std::map<std::string /* diagram name */,
        std::vector<std::string> /*translation units*/>
        translation_units_map;

    // We have to generate the translation units list for each diagram before
    // scheduling tasks, because std::filesystem::current_path cannot be trusted
    // with multiple threads
    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command line,
        // and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        // Get all translation units matching the glob from diagram
        // configuration
        std::vector<std::string> translation_units =
            diagram->get_translation_units(current_directory);

        std::vector<std::string> valid_translation_units{};
        std::copy_if(compilation_database_files.begin(),
            compilation_database_files.end(),
            std::back_inserter(valid_translation_units),
            [&translation_units](const auto &tu) {
                return std::find(translation_units.begin(),
                           translation_units.end(),
                           tu) != translation_units.end();
            });

        translation_units_map[name] = std::move(valid_translation_units);
    }

    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command line,
        // and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        const auto &valid_translation_units = translation_units_map[name];

        if (valid_translation_units.empty()) {
            LOG_ERROR(
                "Diagram {} generation failed: no translation units found",
                name);
            continue;
        }

        futs.emplace_back(generator_executor.add(
            [&od, &name = name, &diagram = diagram, &config = config,
                db = std::ref(*db),
                translation_units = std::move(valid_translation_units),
                verbose]() {
                try {
                    generate_diagram(
                        od, name, diagram, db, translation_units, verbose);
                }
                catch (std::runtime_error &e) {
                    LOG_ERROR(e.what());
                }
            }));
    }

    for (auto &fut : futs) {
        fut.get();
    }

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

bool check_output_directory(const std::string &dir)
{
    namespace fs = std::filesystem;
    using std::cout;

    fs::path output_dir{dir};

    if (fs::exists(output_dir) && !fs::is_directory(output_dir)) {
        cout << "ERROR: " << dir << " is not a directory...\n";
        return false;
    }

    if (!fs::exists(output_dir)) {
        return fs::create_directories(output_dir);
    }

    return true;
}

void print_version()
{
    std::cout << "clang-uml " << clanguml::version::CLANG_UML_VERSION << '\n';
    std::cout << "Copyright (C) 2021-2022 Bartek Kryza <bkryza@gmail.com>"
              << '\n';
    std::cout << "Built with LLVM version: "
              << std::string{BACKEND_PACKAGE_STRING}.substr(5) << std::endl;
    std::cout << "Using LLVM version: " << clang::getClangFullVersion()
              << std::endl;
}

void print_diagrams_list(const clanguml::config::config &cfg)
{
    using std::cout;

    cout << "The following diagrams are defined in the config file:\n";
    for (const auto &[name, diagram] : cfg.diagrams) {
        cout << "  - " << name << " [" << to_string(diagram->type()) << "]";
        cout << '\n';
    }
}
