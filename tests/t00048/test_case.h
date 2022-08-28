/**
 * tests/t00048/test_case.h
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

TEST_CASE("t00048", "[test-case][class]")
{
    auto [config, db] = load_config("t00048");

    auto diagram = config.diagrams["t00048_class"];

    REQUIRE(diagram->name == "t00048_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00048_class");

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    // Check if all classes exist
    REQUIRE_THAT(puml, IsAbstractClass(_A("Base")));
    REQUIRE_THAT(puml, IsClass(_A("A")));
    REQUIRE_THAT(puml, IsClass(_A("B")));

    // Check if class templates exist
    REQUIRE_THAT(puml, IsAbstractClassTemplate("BaseTemplate", "T"));
    REQUIRE_THAT(puml, IsClassTemplate("ATemplate", "T"));
    REQUIRE_THAT(puml, IsClassTemplate("BTemplate", "T"));

    // Check if all inheritance relationships exist
    REQUIRE_THAT(puml, IsBaseClass(_A("Base"), _A("A")));
    REQUIRE_THAT(puml, IsBaseClass(_A("Base"), _A("B")));


    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}