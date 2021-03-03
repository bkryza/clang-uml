/**
 * tests/t00006/test_case.cc
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

TEST_CASE("Test t00006", "[unit-test]")
{
    spdlog::set_level(spdlog::level::debug);

    auto [config, db] = load_config("t00006");

    auto diagram = config.diagrams["t00006_class"];

    REQUIRE(diagram->name == "t00006_class");

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t00006"}));

    REQUIRE(diagram->exclude.namespaces.size() == 0);

    REQUIRE(diagram->should_include("clanguml::t00006::A"));
    REQUIRE(diagram->should_include("clanguml::t00006::B"));
    REQUIRE(diagram->should_include("clanguml::t00006::C"));
    REQUIRE(diagram->should_include("clanguml::t00006::D"));
    REQUIRE(diagram->should_include("clanguml::t00006::E"));

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model.name == "t00006_class");

    auto puml = generate_class_puml(diagram, model);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClass("A"));
    REQUIRE_THAT(puml, IsClass("B"));
    REQUIRE_THAT(puml, IsClass("C"));
    REQUIRE_THAT(puml, IsClass("D"));
    REQUIRE_THAT(puml, IsClass("E"));
    REQUIRE_THAT(puml, IsClass("F"));
    REQUIRE_THAT(puml, IsClass("G"));
    REQUIRE_THAT(puml, IsClass("H"));
    REQUIRE_THAT(puml, IsClass("I"));
    REQUIRE_THAT(puml, IsClass("J"));
    REQUIRE_THAT(puml, IsClass("K"));
    REQUIRE_THAT(puml, IsClass("R"));

    REQUIRE_THAT(puml, IsComposition("R", "A", "a"));
    REQUIRE_THAT(puml, IsAssociation("R", "B", "b"));
    REQUIRE_THAT(puml, IsComposition("R", "C", "c"));
    REQUIRE_THAT(puml, IsAssociation("R", "D", "d"));
    REQUIRE_THAT(puml, IsComposition("R", "E", "e"));
    REQUIRE_THAT(puml, IsComposition("R", "F", "f"));
    REQUIRE_THAT(puml, IsAssociation("R", "G", "g"));
    REQUIRE_THAT(puml, IsComposition("R", "H", "h"));
    REQUIRE_THAT(puml, IsAssociation("R", "I", "i"));
    REQUIRE_THAT(puml, IsComposition("R", "J", "j"));
    REQUIRE_THAT(puml, IsAssociation("R", "K", "k"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
