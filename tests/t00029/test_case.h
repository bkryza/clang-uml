/**
 * tests/t00029/test_case.cc
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

TEST_CASE("t00029", "[test-case][class]")
{
    auto [config, db] = load_config("t00029");

    auto diagram = config.diagrams["t00029_class"];

    REQUIRE(diagram->name == "t00029_class");


    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model.name() == "t00029_class");
    REQUIRE(model.should_include("clanguml::t00029::A"));

    auto puml = generate_class_puml(diagram, model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, IsClass(_A("A")));
    REQUIRE_THAT(puml, !IsClass(_A("B")));
    REQUIRE_THAT(puml, IsClassTemplate("C", "T"));
    REQUIRE_THAT(puml, !IsClassTemplate("D", "T"));
    REQUIRE_THAT(puml, IsEnum(_A("E")));
    REQUIRE_THAT(puml, !IsEnum(_A("F")));
    REQUIRE_THAT(puml, IsClass(_A("G1")));
    REQUIRE_THAT(puml, IsClass(_A("G2")));
    REQUIRE_THAT(puml, IsClass(_A("G3")));
    REQUIRE_THAT(puml, IsClass(_A("G4")));

    REQUIRE_THAT(puml, IsClass(_A("R")));

    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("G1"), "+g1"));
    REQUIRE_THAT(puml, !IsAggregation(_A("R"), _A("G2"), "+g2"));
    REQUIRE_THAT(puml, !IsAggregation(_A("R"), _A("G3"), "+g3"));
    REQUIRE_THAT(puml, IsAssociation(_A("R"), _A("G4"), "+g4"));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
