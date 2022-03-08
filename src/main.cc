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

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "config/config.h"
#include "cx/compilation_database.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"

#include "config/config.h"
#include "util/util.h"

#include <cli11/CLI11.hpp>
#include <cppast/libclang_parser.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string.h>

using namespace clanguml;
using config::config;
using cx::compilation_database;

void print_diagrams_list(const clanguml::config::config &cfg);

bool check_output_directory(const std::string &dir);

int main(int argc, const char *argv[])
{
    CLI::App app{"Clang-based PlantUML diagram generator for C++"};

    std::string config_path{".clang-uml"};
    std::string compilation_database_dir{'.'};
    std::vector<std::string> diagram_names{};
    std::optional<std::string> output_directory;
    bool verbose{false};
    bool list_diagrams{false};

    app.add_option(
        "-c,--config", config_path, "Location of configuration file");
    app.add_option("-d,--compile-database", compilation_database_dir,
        "Location of compilation database directory");
    app.add_option("-n,--diagram-name", diagram_names,
        "List of diagram names to generate");
    app.add_option("-o,--output-directory", output_directory,
        "Override output directory specified in config file");
    app.add_flag("-v,--verbose", verbose, "Verbose logging");
    app.add_flag("-l,--list-diagrams", list_diagrams,
        "Print list of diagrams defined in the config file");

    CLI11_PARSE(app, argc, argv);

    if (verbose) {
        spdlog::default_logger_raw()->set_level(spdlog::level::debug);
        spdlog::default_logger_raw()->set_pattern("[%l] %v");
    }

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

    cppast::libclang_compilation_database db(config.compilation_database_dir());

    auto od = config.output_directory();
    if (output_directory)
        od = output_directory.value();

    if (!check_output_directory(od))
        return 1;

    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command line,
        // and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        using clanguml::config::class_diagram;
        using clanguml::config::diagram_type;
        using clanguml::config::package_diagram;
        using clanguml::config::sequence_diagram;

        auto path = std::filesystem::path{od} / fmt::format("{}.puml", name);
        std::ofstream ofs;
        ofs.open(path, std::ofstream::out | std::ofstream::trunc);

        if (diagram->type() == diagram_type::class_diagram) {
            using diagram_config = clanguml::config::class_diagram;
            using diagram_model = clanguml::class_diagram::model::diagram;
            using diagram_visitor =
                clanguml::class_diagram::visitor::translation_unit_visitor;

            auto model =
                clanguml::common::generators::plantuml::generate<diagram_model,
                    diagram_config, diagram_visitor>(db, diagram->name,
                    dynamic_cast<diagram_config &>(*diagram));

            ofs << clanguml::class_diagram::generators::plantuml::generator(
                dynamic_cast<diagram_config &>(*diagram), model);
        }
        else if (diagram->type() == diagram_type::sequence_diagram) {
            using diagram_config = clanguml::config::sequence_diagram;
            using diagram_model = clanguml::sequence_diagram::model::diagram;
            using diagram_visitor =
                clanguml::sequence_diagram::visitor::translation_unit_visitor;

            auto model =
                clanguml::common::generators::plantuml::generate<diagram_model,
                    diagram_config, diagram_visitor>(db, diagram->name,
                    dynamic_cast<diagram_config &>(*diagram));

            ofs << clanguml::sequence_diagram::generators::plantuml::generator(
                dynamic_cast<clanguml::config::sequence_diagram &>(*diagram),
                model);
        }
        else if (diagram->type() == diagram_type::package_diagram) {
            using diagram_config = clanguml::config::package_diagram;
            using diagram_model = clanguml::package_diagram::model::diagram;
            using diagram_visitor =
                clanguml::package_diagram::visitor::translation_unit_visitor;

            auto model =
                clanguml::common::generators::plantuml::generate<diagram_model,
                    diagram_config, diagram_visitor>(db, diagram->name,
                    dynamic_cast<diagram_config &>(*diagram));

            ofs << clanguml::package_diagram::generators::plantuml::generator(
                dynamic_cast<diagram_config &>(*diagram), model);
        }

        LOG_INFO("Written {} diagram to {}", name, path.string());

        ofs.close();
    }

    return 0;
}

bool check_output_directory(const std::string &dir)
{
    namespace fs = std::filesystem;
    using std::cout;

    fs::path output_dir{dir};

    if (!fs::exists(output_dir)) {
        cout << "ERROR: Output directory " << dir << " doesn't exist...\n";
        return false;
    }

    if (!fs::is_directory(output_dir)) {
        cout << "ERROR: " << dir << " is not a directory...\n";
        return false;
    }

    return true;
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
