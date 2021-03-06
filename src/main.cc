/**
 * src/main.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

#include "config/config.h"
#include "cx/compilation_database.h"
#include "puml/class_diagram_generator.h"
#include "puml/sequence_diagram_generator.h"
#include "uml/class_diagram_model.h"
#include "uml/class_diagram_visitor.h"
#include "uml/sequence_diagram_visitor.h"

#include <cli11/CLI11.hpp>
#include <glob/glob.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace clanguml;
using config::config;
using cx::compilation_database;

int main(int argc, const char *argv[])
{
    spdlog::set_pattern("[%l] %v");

    CLI::App app{"Clang-based PlantUML generator from C++ sources"};

    std::string config_path{".clanguml"};
    std::string compilation_database_dir{'.'};
    bool verbose{false};

    app.add_option(
        "-c,--config", config_path, "Location of configuration file");
    app.add_option("-d,--compile-database", compilation_database_dir,
        "Location of configuration file");
    app.add_flag("-v,--verbose", verbose, "Verbose logging");

    CLI11_PARSE(app, argc, argv);

    if (verbose)
        spdlog::set_level(spdlog::level::debug);

    spdlog::info("Loading clang-uml config from {}", config_path);

    auto config = clanguml::config::load(config_path);

    spdlog::info("Loading compilation database from {} directory",
        config.compilation_database_dir);

    auto db =
        compilation_database::from_directory(config.compilation_database_dir);

    for (const auto &[name, diagram] : config.diagrams) {
        using clanguml::config::class_diagram;
        using clanguml::config::sequence_diagram;

        std::filesystem::path path{"puml/" + name + ".puml"};
        std::ofstream ofs;
        ofs.open(path, std::ofstream::out | std::ofstream::trunc);

        if (std::dynamic_pointer_cast<class_diagram>(diagram)) {
            auto model = generators::class_diagram::generate(
                db, name, dynamic_cast<class_diagram &>(*diagram));

            ofs << clanguml::generators::class_diagram::puml::generator(
                dynamic_cast<clanguml::config::class_diagram &>(*diagram),
                model);
        }
        else if (std::dynamic_pointer_cast<sequence_diagram>(diagram)) {
            auto model = generators::sequence_diagram::generate(
                db, name, dynamic_cast<sequence_diagram &>(*diagram));

            ofs << clanguml::generators::sequence_diagram::puml::generator(
                dynamic_cast<clanguml::config::sequence_diagram &>(*diagram),
                model);
        }

        ofs.close();
    }

    return 0;
}
