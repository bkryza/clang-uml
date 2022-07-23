/**
 * tests/t00027/test_case.cc
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

TEST_CASE("t00027", "[test-case][class]")
{
    auto [config, db] = load_config("t00027");

    auto diagram = config.diagrams["t00027_class"];

    REQUIRE(diagram->name == "t00027_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00027_class");
    REQUIRE(model->should_include("clanguml::t00027::A"));

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsAbstractClass(_A("Shape")));
    REQUIRE_THAT(puml, IsAbstractClass(_A("ShapeDecorator")));
    REQUIRE_THAT(puml, IsClassTemplate("Line", "T<>..."));
    REQUIRE_THAT(puml, IsClassTemplate("Text", "T<>..."));
    REQUIRE_THAT(puml, IsInstantiation(_A("Line<T<>...>"), _A("Line<Color>")));
    REQUIRE_THAT(
        puml, IsInstantiation(_A("Line<T<>...>"), _A("Line<Color,Weight>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("Text<T<>...>"), _A("Text<Color>")));
    REQUIRE_THAT(
        puml, IsInstantiation(_A("Text<T<>...>"), _A("Text<Color,Weight>")));

    REQUIRE_THAT(
        puml, IsAggregation(_A("Window"), _A("Line<Color,Weight>"), "+border"));
    REQUIRE_THAT(
        puml, IsAggregation(_A("Window"), _A("Line<Color>"), "+divider"));
    REQUIRE_THAT(
        puml, IsAggregation(_A("Window"), _A("Text<Color,Weight>"), "+title"));
    REQUIRE_THAT(
        puml, IsAggregation(_A("Window"), _A("Text<Color>"), "+description"));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
