/**
 * tests/t00041/test_case.cc
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

TEST_CASE("t00041", "[test-case][class]")
{
    auto [config, db] = load_config("t00041");

    auto diagram = config.diagrams["t00041_class"];

    REQUIRE(diagram->name == "t00041_class");
    REQUIRE(diagram->generate_packages() == false);

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model->name() == "t00041_class");

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, !IsClass(_A("A")));
    REQUIRE_THAT(puml, !IsClass(_A("AA")));
    REQUIRE_THAT(puml, !IsClass(_A("AAA")));

    REQUIRE_THAT(puml, !IsClass(_A("B")));

    REQUIRE_THAT(puml, IsClass(_A("D")));
    REQUIRE_THAT(puml, IsClass(_A("E")));
    REQUIRE_THAT(puml, IsClass(_A("F")));
    REQUIRE_THAT(puml, IsClass(_A("R")));
    REQUIRE_THAT(puml, IsClass(_A("RR")));
    REQUIRE_THAT(puml, IsClass(_A("RRR")));

    REQUIRE_THAT(puml, IsBaseClass(_A("R"), _A("RR")));
    REQUIRE_THAT(puml, IsBaseClass(_A("RR"), _A("RRR")));

    REQUIRE_THAT(puml, IsAssociation(_A("D"), _A("RR"), "+rr"));
    REQUIRE_THAT(puml, IsAssociation(_A("RR"), _A("E"), "+e"));
    REQUIRE_THAT(puml, IsAssociation(_A("RR"), _A("F"), "+f"));

    REQUIRE_THAT(puml, IsClass(_A("ns1::N")));
    REQUIRE_THAT(puml, IsClass(_A("ns1::NN")));
    REQUIRE_THAT(puml, IsClass(_A("ns1::NM")));
    REQUIRE_THAT(puml, IsBaseClass(_A("ns1::N"), _A("ns1::NN")));
    REQUIRE_THAT(puml, IsBaseClass(_A("ns1::N"), _A("ns1::NM")));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
