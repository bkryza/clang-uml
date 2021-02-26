/**
 * tests/test_cases.cc
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
    std::filesystem::create_directory(p.parent_path());
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
    REQUIRE_THAT(puml, Contains("abstract A"));
    REQUIRE_THAT(puml, Contains("class B"));
    REQUIRE_THAT(puml, Contains("class C"));
    REQUIRE_THAT(puml, Contains("class D"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}

TEST_CASE("Test t00003", "[unit-test]")
{
    spdlog::set_level(spdlog::level::debug);

    auto [config, db] = load_config("t00003");

    auto diagram = config.diagrams["t00003_class"];

    REQUIRE(diagram->name == "t00003_class");

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t00003"}));

    REQUIRE(diagram->exclude.namespaces.size() == 0);

    REQUIRE(diagram->should_include("clanguml::t00003::A"));

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model.name == "t00003_class");

    auto puml = generate_class_puml(diagram, model);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, Contains("class A"));

    REQUIRE_THAT(puml, Contains("+ A() = default"));
    REQUIRE_THAT(puml, Contains("+ A() = default"));
    REQUIRE_THAT(puml, Contains("+ A() = default"));
    REQUIRE_THAT(puml, Contains("+ ~A() = default"));
    REQUIRE_THAT(puml, Contains("+ basic_method()"));
    REQUIRE_THAT(puml, Contains("{static} +int static_method()"));
    REQUIRE_THAT(puml, Contains("+ const_method() const"));
    REQUIRE_THAT(puml, Contains("# protected_method()"));
    REQUIRE_THAT(puml, Contains("- private_method()"));
    REQUIRE_THAT(puml, Contains("+int public_member"));
    REQUIRE_THAT(puml, Contains("#int protected_member"));
    REQUIRE_THAT(puml, Contains("-int private_member"));
    REQUIRE_THAT(puml, Contains("-int a"));
    REQUIRE_THAT(puml, Contains("-int b"));
    REQUIRE_THAT(puml, Contains("-int c"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}

TEST_CASE("Test t00004", "[unit-test]")
{
    spdlog::set_level(spdlog::level::debug);

    auto [config, db] = load_config("t00004");

    auto diagram = config.diagrams["t00004_class"];

    REQUIRE(diagram->name == "t00004_class");

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t00004"}));

    REQUIRE(diagram->exclude.namespaces.size() == 0);

    REQUIRE(diagram->should_include("clanguml::t00004::A"));
    REQUIRE(diagram->should_include("clanguml::t00004::A::AA"));
    REQUIRE(diagram->should_include("clanguml::t00004::A:::AAA"));

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model.name == "t00004_class");

    auto puml = generate_class_puml(diagram, model);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, Contains("class A"));
    REQUIRE_THAT(puml, Contains("A +-- A::AA"));
    REQUIRE_THAT(puml, Contains("A::AA +-- A::AA::AAA"));
    REQUIRE_THAT(puml, Contains("A::AA +-- A::AA::Lights"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
