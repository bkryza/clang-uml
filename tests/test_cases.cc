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

using Catch::Matchers::Contains;
using Catch::Matchers::EndsWith;
using Catch::Matchers::StartsWith;
using Catch::Matchers::VectorContains;
using clanguml::cx::compilation_database;

auto load_config(const std::string &test_name)
{
    auto config = clanguml::config::load(test_name + "/.clanguml");

    auto db = clanguml::cx::compilation_database::from_directory(
        config.compilation_database_dir);

    return std::make_pair(std::move(config), std::move(db));
}

auto generate_sequence_diagram(compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    auto diagram_model =
        clanguml::generators::sequence_diagram::generate(db, diagram->name,
            dynamic_cast<clanguml::config::sequence_diagram &>(*diagram));

    return diagram_model;
}

auto generate_class_diagram(compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    auto diagram_model =
        clanguml::generators::class_diagram::generate(db, diagram->name,
            dynamic_cast<clanguml::config::class_diagram &>(*diagram));

    return diagram_model;
}

std::string generate_sequence_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::model::sequence_diagram::diagram &model)
{
    using namespace clanguml::generators::sequence_diagram::puml;

    std::stringstream ss;

    ss << generator(
        dynamic_cast<clanguml::config::sequence_diagram &>(*config), model);

    return ss.str();
}

std::string generate_class_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::model::class_diagram::diagram &model)
{
    using namespace clanguml::generators::class_diagram::puml;

    std::stringstream ss;

    ss << generator(
        dynamic_cast<clanguml::config::class_diagram &>(*config), model);

    return ss.str();
}

void save_puml(const std::string &path, const std::string &puml)
{
    std::filesystem::path p{path};
    spdlog::error("PWD: {}", std::filesystem::current_path().string());
    spdlog::error("SAVING TEST PWD {} DIAGRAM: {}", p.string());
    std::ofstream ofs;
    ofs.open(p, std::ofstream::out | std::ofstream::trunc);
    ofs << puml;
    ofs.close();
}

TEST_CASE("Test t00001", "[unit-test]")
{
    spdlog::set_level(spdlog::level::debug);

    auto [config, db] = load_config("t00001");

    auto diagram = config.diagrams["t00001_sequence"];

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t00001"}));

    REQUIRE(diagram->exclude.namespaces.size() == 1);
    REQUIRE_THAT(diagram->exclude.namespaces,
        VectorContains(std::string{"clanguml::t00001::detail"}));

    REQUIRE(diagram->should_include("clanguml::t00001::A"));
    REQUIRE(!diagram->should_include("clanguml::t00001::detail::C"));
    REQUIRE(!diagram->should_include("std::vector"));

    REQUIRE(diagram->name == "t00001_sequence");

    auto model = generate_sequence_diagram(db, diagram);

    REQUIRE(model.name == "t00001_sequence");

    auto puml = generate_sequence_puml(diagram, model);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}

TEST_CASE("Test t00002", "[unit-test]")
{
    spdlog::set_level(spdlog::level::debug);

    auto [config, db] = load_config("t00002");

    auto diagram = config.diagrams["t00002_class"];

    REQUIRE(diagram->name == "t00002_class");

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t00002"}));

    REQUIRE(diagram->exclude.namespaces.size() == 0);

    REQUIRE(diagram->should_include("clanguml::t00002::A"));
    REQUIRE(!diagram->should_include("std::vector"));

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model.name == "t00002_class");

    auto puml = generate_class_puml(diagram, model);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
