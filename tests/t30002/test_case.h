/**
 * tests/t30002/test_case.cc
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

TEST_CASE("t30002", "[test-case][package]")
{
    auto [config, db] = load_config("t30002");

    auto diagram = config.diagrams["t30002_package"];

    REQUIRE(diagram->name == "t30002_package");

    auto model = generate_package_diagram(db, diagram);

    REQUIRE(model->name() == "t30002_package");

    auto puml = generate_package_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, IsPackage("A1"));
    REQUIRE_THAT(puml, IsPackage("A2"));
    REQUIRE_THAT(puml, IsPackage("A3"));
    REQUIRE_THAT(puml, IsPackage("A4"));
    REQUIRE_THAT(puml, IsPackage("A5"));
    REQUIRE_THAT(puml, IsPackage("A6"));
    REQUIRE_THAT(puml, IsPackage("A7"));
    REQUIRE_THAT(puml, IsPackage("A8"));
    REQUIRE_THAT(puml, IsPackage("A9"));
    REQUIRE_THAT(puml, IsPackage("A11"));
    REQUIRE_THAT(puml, IsPackage("A12"));
    REQUIRE_THAT(puml, IsPackage("A13"));
    REQUIRE_THAT(puml, IsPackage("A14"));
    REQUIRE_THAT(puml, IsPackage("A15"));

    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A1")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A2")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A3")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A4")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A5")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A6")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A7")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A8")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A9")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A10")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A11")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A12")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A13")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A14")));
    REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A15")));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
