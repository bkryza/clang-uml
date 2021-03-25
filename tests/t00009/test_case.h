/**
 * tests/t00009/test_case.cc
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

TEST_CASE("t00009", "[test-case][class]")
{
    auto [config, db] = load_config("t00009");

    auto diagram = config.diagrams["t00009_class"];

    REQUIRE(diagram->name == "t00009_class");

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t00009"}));

    REQUIRE(diagram->exclude.namespaces.size() == 0);

    REQUIRE(diagram->should_include("clanguml::t00009::A"));
    REQUIRE(diagram->should_include("clanguml::t00009::B"));

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model.name == "t00009_class");

    auto puml = generate_class_puml(diagram, model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "T"));
    REQUIRE_THAT(puml, IsClass(_A("B")));

    REQUIRE_THAT(puml, IsField(Public("T value")));
    REQUIRE_THAT(puml, IsField(Public("A<int> aint")));
    REQUIRE_THAT(puml, IsField(Public("A<std::string>* astring")));
    REQUIRE_THAT(
        puml, IsField(Public("A<std::vector<std::string>>& avector")));

    REQUIRE_THAT(puml, IsInstantiation(_A("A<T>"), _A("A<int>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("A<T>"), _A("A<std::string>")));

    REQUIRE_THAT(puml, IsComposition(_A("B"), _A("A<int>"), "aint"));
    REQUIRE_THAT(puml, IsAssociation(_A("B"), _A("A<std::string>"), "astring"));
    REQUIRE_THAT(puml,
        IsAssociation(_A("B"), _A("A<std::vector<std::string>>"), "avector"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
