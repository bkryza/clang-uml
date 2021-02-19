#define CATCH_CONFIG_MAIN

#include "config/config.h"
#include "cx/compilation_database.h"
#include "puml/class_diagram_generator.h"
#include "puml/sequence_diagram_generator.h"
#include "uml/class_diagram_model.h"
#include "uml/class_diagram_visitor.h"
#include "uml/sequence_diagram_visitor.h"

#include "catch.h"
#include <complex>
#include <filesystem>
#include <string>

TEST_CASE("Test t00001", "[unit-test]")
{
    spdlog::set_level(spdlog::level::debug);

    // REQUIRE(2 + 5 == 7);
    std::cout << "RUNNING TEST IN " << std::filesystem::current_path()
              << std::endl;

    auto config = clanguml::config::load("t00001/.clanguml");

    auto db = clanguml::cx::compilation_database::from_directory(
        config.compilation_database_dir);

    auto diagram = config.diagrams["t00001_sequence"];
    auto diagram_model =
        clanguml::generators::sequence_diagram::generate(db, "t00001_sequence",
            dynamic_cast<clanguml::config::sequence_diagram &>(*diagram));

    REQUIRE(diagram_model.name == "t00001_sequence");

    auto generator = clanguml::generators::sequence_diagram::puml::generator(
        dynamic_cast<clanguml::config::sequence_diagram &>(*diagram), diagram_model);
    std::cout << generator;
}
