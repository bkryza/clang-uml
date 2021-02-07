#include "config/config.h"
#include "puml/class_diagram_generator.h"
#include "uml/class_diagram_model.h"
#include "uml/class_diagram_visitor.h"
#include "uml/compilation_database.h"

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

using clanguml::config::config;
using clanguml::cx::compilation_database;

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
        spdlog::info("Generating diagram {}.puml", name);
        clanguml::model::class_diagram::diagram d;
        d.name = name;

        // Get all translation units matching the glob from diagram
        // configuration
        std::vector<std::filesystem::path> translation_units{};
        for (const auto &g : diagram->glob) {
            spdlog::debug("Processing glob: {}", g);
            const auto matches = glob::glob(g);
            std::copy(matches.begin(), matches.end(),
                std::back_inserter(translation_units));
        }

        // Process all matching translation units
        for (const auto &tu_path : translation_units) {
            spdlog::debug("Processing translation unit: {}",
                std::filesystem::canonical(tu_path).c_str());

            auto tu = db.parse_translation_unit(tu_path);

            auto cursor = clang_getTranslationUnitCursor(tu);

            if (clang_Cursor_isNull(cursor)) {
                spdlog::debug("CURSOR IS NULL");
            }

            spdlog::debug("Cursor kind: {}",
                clang_getCString(clang_getCursorKindSpelling(cursor.kind)));
            spdlog::debug("Cursor name: {}",
                clang_getCString(clang_getCursorDisplayName(cursor)));

            clanguml::visitor::class_diagram::tu_context ctx(d);
            auto res = clang_visitChildren(cursor,
                clanguml::visitor::class_diagram::translation_unit_visitor,
                &ctx);

            spdlog::debug("Processing result: {}", res);
        }

        std::filesystem::path path{"puml/" + d.name + ".puml"};
        std::ofstream ofs;
        ofs.open(path, std::ofstream::out | std::ofstream::trunc);

        auto generator = clanguml::generators::class_diagram::puml::generator(
            dynamic_cast<clanguml::config::class_diagram &>(*diagram), d);

        ofs << generator;

        ofs.close();
    }
    return 0;
}
