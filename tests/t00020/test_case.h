/**
 * tests/t00020/test_case.cc
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

TEST_CASE("t00020", "[test-case][class]")
{
    auto [config, db] = load_config("t00020");

    auto diagram = config.diagrams["t00020_class"];

    REQUIRE(diagram->name == "t00020_class");

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model->name() == "t00020_class");
    REQUIRE(model->should_include("clanguml::t00020::ProductA"));

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsAbstractClass(_A("ProductA")));
    REQUIRE_THAT(puml, IsAbstractClass(_A("ProductB")));
    REQUIRE_THAT(puml, IsClass(_A("ProductA1")));
    REQUIRE_THAT(puml, IsClass(_A("ProductA2")));
    REQUIRE_THAT(puml, IsClass(_A("ProductB1")));
    REQUIRE_THAT(puml, IsClass(_A("ProductB2")));
    REQUIRE_THAT(puml, IsAbstractClass(_A("AbstractFactory")));
    REQUIRE_THAT(puml, IsClass(_A("Factory1")));
    REQUIRE_THAT(puml, IsClass(_A("Factory2")));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
