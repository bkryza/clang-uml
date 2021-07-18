/**
 * tests/t00013/test_case.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t00013", "[test-case][class]")
{
    auto [config, db] = load_config("t00013");

    auto diagram = config.diagrams["t00013_class"];

    REQUIRE(diagram->name == "t00013_class");

    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t00013"}));

    REQUIRE(diagram->exclude.namespaces.size() == 0);

    REQUIRE(diagram->should_include("clanguml::t00013::A"));
    REQUIRE(diagram->should_include("clanguml::t00013::B"));
    REQUIRE(diagram->should_include("ABCD::F"));

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model.name == "t00013_class");

    auto puml = generate_class_puml(diagram, model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClass(_A("A")));
    REQUIRE_THAT(puml, IsClass(_A("B")));
    REQUIRE_THAT(puml, IsClass(_A("C")));
    REQUIRE_THAT(puml, IsClass(_A("D")));
    REQUIRE_THAT(puml, !IsDependency(_A("R"), _A("R")));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("A")));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("B")));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("C")));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("D")));
    REQUIRE_THAT(puml, IsDependency(_A("D"), _A("R")));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("E<T>")));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("E<int>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("E<T>"), _A("E<int>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("E<T>"), _A("E<std::string>")));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("E<std::string>"), "estring"));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("ABCD::F<T>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("ABCD::F<T>"), _A("ABCD::F<int>")));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("ABCD::F<int>")));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
