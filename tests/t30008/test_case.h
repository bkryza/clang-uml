/**
 * tests/t30008/test_case.cc
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

TEST_CASE("t30008", "[test-case][package]")
{
    auto [config, db] = load_config("t30008");

    auto diagram = config.diagrams["t30008_package"];

    REQUIRE(diagram->name == "t30008_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30008_package");

    auto puml = generate_package_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, IsPackage("A"));
    REQUIRE_THAT(puml, IsPackage("B"));
    REQUIRE_THAT(puml, IsPackage("C"));
    REQUIRE_THAT(puml, !IsPackage("X"));

    REQUIRE_THAT(puml, IsDependency(_A("B"), _A("A")));
    REQUIRE_THAT(puml, IsDependency(_A("C"), _A("B")));

    REQUIRE_THAT(puml, IsPackage("D"));
    REQUIRE_THAT(puml, IsPackage("E"));
    REQUIRE_THAT(puml, IsPackage("F"));
    REQUIRE_THAT(puml, !IsPackage("Y"));

    REQUIRE_THAT(puml, IsDependency(_A("E"), _A("D")));
    REQUIRE_THAT(puml, IsDependency(_A("F"), _A("E")));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
