/**
 * tests/t00019/test_case.cc
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

TEST_CASE("t00019", "[test-case][class]")
{
    auto [config, db] = load_config("t00019");

    auto diagram = config.diagrams["t00019_class"];

    REQUIRE(diagram->name == "t00019_class");

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model->name() == "t00019_class");

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClass(_A("Base")));
    REQUIRE_THAT(puml, IsClassTemplate("Layer1", "LowerLayer"));
    REQUIRE_THAT(puml, IsClassTemplate("Layer2", "LowerLayer"));
    REQUIRE_THAT(puml, IsClassTemplate("Layer3", "LowerLayer"));
    REQUIRE_THAT(puml, IsBaseClass(_A("Base"), _A("Layer3<Base>")));
    REQUIRE_THAT(
        puml, IsBaseClass(_A("Layer3<Base>"), _A("Layer2<Layer3<Base>>")));
    REQUIRE_THAT(puml,
        IsBaseClass(
            _A("Layer2<Layer3<Base>>"), _A("Layer1<Layer2<Layer3<Base>>>")));

    REQUIRE_THAT(puml,
        IsAggregation(_A("A"), _A("Layer1<Layer2<Layer3<Base>>>"), "+layers"));

    REQUIRE_THAT(
        puml, !IsAggregation(_A("A"), _A("Layer2<Layer3<Base>>"), "+layers"));

    REQUIRE_THAT(puml, !IsAggregation(_A("A"), _A("Layer3<Base>"), "+layers"));

    REQUIRE_THAT(puml, !IsAggregation(_A("A"), _A("Base"), "+layers"));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
