/**
 * tests/t00010/test_case.cc
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

TEST_CASE("t00010", "[test-case][class]")
{
    auto [config, db] = load_config("t00010");

    auto diagram = config.diagrams["t00010_class"];

    REQUIRE(diagram->name == "t00010_class");

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t00010"}));

    REQUIRE(diagram->exclude.namespaces.size() == 0);

    REQUIRE(diagram->should_include("clanguml::t00010::A"));
    REQUIRE(diagram->should_include("clanguml::t00010::B"));

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model.name == "t00010_class");

    auto puml = generate_class_puml(diagram, model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "T,P"));
    REQUIRE_THAT(puml, IsClassTemplate("B", "T"));

    REQUIRE_THAT(puml, IsField(Public("A<T,std::string> astring")));
    REQUIRE_THAT(puml, IsField(Public("B<int> aintstring")));

    REQUIRE_THAT(puml, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("B<T>"), _A("B<int>")));

    REQUIRE_THAT(
        puml, IsComposition(_A("B<T>"), _A("A<T,std::string>"), "astring"));
    REQUIRE_THAT(puml, IsComposition(_A("C"), _A("B<int>"), "aintstring"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
