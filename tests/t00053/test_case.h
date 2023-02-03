/**
 * tests/t00053/test_case.h
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

TEST_CASE("t00053", "[test-case][class]")
{
    auto [config, db] = load_config("t00053");

    auto diagram = config.diagrams["t00053_class"];

    REQUIRE(diagram->name == "t00053_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00053_class");

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    // Check if all classes exist
    REQUIRE_THAT(puml, IsClass(_A("a")));
    REQUIRE_THAT(puml, IsClass(_A("b")));
    REQUIRE_THAT(puml, IsClass(_A("c")));
    REQUIRE_THAT(puml, IsClass(_A("d")));
    REQUIRE_THAT(puml, IsClass(_A("e")));
    REQUIRE_THAT(puml, IsClass(_A("f")));
    REQUIRE_THAT(puml, IsClass(_A("g")));

    REQUIRE_THAT(puml, IsClass(_A("A")));
    REQUIRE_THAT(puml, IsClass(_A("B")));
    REQUIRE_THAT(puml, IsClass(_A("C")));
    REQUIRE_THAT(puml, IsClass(_A("D")));
    REQUIRE_THAT(puml, IsClass(_A("E")));
    REQUIRE_THAT(puml, IsClass(_A("F")));
    REQUIRE_THAT(puml, IsClass(_A("G")));

    save_puml(config.output_directory() + "/" + diagram->name + ".puml", puml);
}