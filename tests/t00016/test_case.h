/**
 * tests/t00016/test_case.cc
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

TEST_CASE("t00016", "[test-case][class]")
{
    auto [config, db] = load_config("t00016");

    auto diagram = config.diagrams["t00016_class"];

    REQUIRE(diagram->name == "t00016_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00016_class");
    REQUIRE(model->should_include("clanguml::t00016::is_numeric"));

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClassTemplate("is_numeric", ""));
    REQUIRE_THAT(puml, IsClassTemplate("is_numeric", "int"));
    REQUIRE_THAT(puml, IsClassTemplate("is_numeric", "bool"));
    REQUIRE_THAT(puml, IsClassTemplate("is_numeric", "char"));
    REQUIRE_THAT(puml, IsClassTemplate("is_numeric", "unsigned char"));

    REQUIRE_THAT(
        puml, IsInstantiation(_A("is_numeric<>"), _A("is_numeric<int>")));
    REQUIRE_THAT(
        puml, IsInstantiation(_A("is_numeric<>"), _A("is_numeric<bool>")));
    REQUIRE_THAT(
        puml, IsInstantiation(_A("is_numeric<>"), _A("is_numeric<char>")));
    REQUIRE_THAT(puml,
        IsInstantiation(_A("is_numeric<>"), _A("is_numeric<unsigned char>")));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
