/**
 * tests/t30009/test_case.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t30009", "[test-case][package]")
{
    auto [config, db] = load_config("t30009");

    auto diagram = config.diagrams["t30009_package"];

    REQUIRE(diagram->name == "t30009_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30009_package");

    auto puml = generate_package_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    // Check if all packages exist
    REQUIRE_THAT(puml, IsPackage("One"));
    REQUIRE_THAT(puml, IsPackage("Two"));
    REQUIRE_THAT(puml, IsPackage("A"));
    REQUIRE_THAT(puml, IsPackage("B"));
    REQUIRE_THAT(puml, IsPackage("C"));
    REQUIRE_THAT(puml, IsPackage("D"));

    save_puml(config.output_directory() + "/" + diagram->name + ".puml", puml);
}