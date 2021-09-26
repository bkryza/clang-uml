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

TEST_CASE("t00006", "[test-case][class]")
{
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

    REQUIRE(model.name() == "t00006_class");

    auto puml = generate_class_puml(diagram, model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClass(_A("A")));
    REQUIRE_THAT(puml, IsClass(_A("B")));
    REQUIRE_THAT(puml, IsClass(_A("C")));
    REQUIRE_THAT(puml, IsClass(_A("D")));
    REQUIRE_THAT(puml, IsClass(_A("E")));
    REQUIRE_THAT(puml, IsClass(_A("F")));
    REQUIRE_THAT(puml, IsClass(_A("G")));
    REQUIRE_THAT(puml, IsClass(_A("H")));
    REQUIRE_THAT(puml, IsClass(_A("I")));
    REQUIRE_THAT(puml, IsClass(_A("J")));
    REQUIRE_THAT(puml, IsClass(_A("K")));
    REQUIRE_THAT(puml, IsClass(_A("L")));
    REQUIRE_THAT(puml, IsClass(_A("M")));
    REQUIRE_THAT(puml, IsClass(_A("N")));
    REQUIRE_THAT(puml, IsClass(_A("NN")));
    REQUIRE_THAT(puml, IsClass(_A("NNN")));

    REQUIRE_THAT(puml,
        IsInstantiation(_A("custom_container<T>"), _A("custom_container<E>")));

    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("A"), "+a"));
    REQUIRE_THAT(puml, IsAssociation(_A("R"), _A("B"), "+b"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("C"), "+c"));
    REQUIRE_THAT(puml, IsAssociation(_A("R"), _A("D"), "+d"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("custom_container<E>"), "+e"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("F"), "+f"));
    REQUIRE_THAT(puml, IsAssociation(_A("R"), _A("G"), "+g"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("H"), "+h"));
    REQUIRE_THAT(puml, IsAssociation(_A("R"), _A("I"), "+i"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("J"), "+j"));
    REQUIRE_THAT(puml, IsAssociation(_A("R"), _A("K"), "+k"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("L"), "+lm"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("M"), "+lm"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("N"), "+ns"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("NN"), "+ns"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("NNN"), "+ns"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
