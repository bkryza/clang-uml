/**
 * tests/t30003/test_case.cc
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

TEST_CASE("t30003", "[test-case][package]")
{
    auto [config, db] = load_config("t30003");

    auto diagram = config.diagrams["t30003_package"];

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t30003"}));

    REQUIRE(diagram->should_include("clanguml::t30003::A"));
    REQUIRE(diagram->should_include("clanguml::t30003::C"));
    REQUIRE(!diagram->should_include("std::vector"));

    REQUIRE(diagram->name == "t30003_package");

    auto model = generate_package_diagram(db, diagram);

    REQUIRE(model.name() == "t30003_package");

    auto puml = generate_package_puml(diagram, model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, IsPackage("ns1"));
    REQUIRE_THAT(puml, IsPackage("ns2"));
    REQUIRE_THAT(puml, IsPackage("ns3"));
    REQUIRE_THAT(puml, IsPackage("ns2_v1_0_0"));
    REQUIRE_THAT(puml, IsPackage("ns2_v0_9_0"));

    REQUIRE_THAT(puml, IsDeprecated(_A("ns2_v0_9_0")));
    REQUIRE_THAT(puml, IsDeprecated(_A("ns3")));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
