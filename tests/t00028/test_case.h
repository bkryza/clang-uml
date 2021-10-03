/**
 * tests/t00028/test_case.cc
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

TEST_CASE("t00028", "[test-case][class]")
{
    auto [config, db] = load_config("t00028");

    auto diagram = config.diagrams["t00028_class"];

    REQUIRE(diagram->name == "t00028_class");

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t00028"}));

    REQUIRE(diagram->exclude.namespaces.size() == 0);

    REQUIRE(diagram->should_include("clanguml::t00028::A"));

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model.name() == "t00028_class");

    auto puml = generate_class_puml(diagram, model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, IsClass(_A("A")));
    REQUIRE_THAT(puml, IsClass(_A("B")));
    REQUIRE_THAT(puml, IsClass(_A("C")));
    REQUIRE_THAT(puml, IsClass(_A("D")));
    REQUIRE_THAT(puml, IsClassTemplate("E", "T"));
    REQUIRE_THAT(puml, IsEnum(_A("F")));
    REQUIRE_THAT(puml, IsClass(_A("R")));
    REQUIRE_THAT(puml, HasNote(_A("A"), "top", "A class note."));
    REQUIRE_THAT(puml, HasNote(_A("B"), "left", "B class note."));
    REQUIRE_THAT(puml, HasNote(_A("C"), "bottom", "C class note."));
    const auto d_note = R"(
D
class
note.)";
    REQUIRE_THAT(puml, HasNote(_A("D"), "left", d_note));
    REQUIRE_THAT(puml, HasNote(_A("E<T>"), "left", "E template class note."));
    REQUIRE_THAT(puml, HasNote(_A("F"), "bottom", "F enum note."));
    REQUIRE_THAT(puml, !HasNote(_A("G"), "left", "G class note."));
    REQUIRE_THAT(puml, HasNote(_A("R"), "right", "R class note."));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
