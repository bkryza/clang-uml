/**
 * tests/t00017/test_case.cc
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

TEST_CASE("t00017", "[test-case][class]")
{
    auto [config, db] = load_config("t00017");

    auto diagram = config.diagrams["t00017_class"];

    REQUIRE(diagram->name == "t00017_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00017_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, IsClass(_A("E")));
        REQUIRE_THAT(src, IsClass(_A("F")));
        REQUIRE_THAT(src, IsClass(_A("G")));
        REQUIRE_THAT(src, IsClass(_A("H")));
        REQUIRE_THAT(src, IsClass(_A("I")));
        REQUIRE_THAT(src, IsClass(_A("J")));
        REQUIRE_THAT(src, IsClass(_A("K")));
        REQUIRE_THAT(src, IsClass(_A("R")));

        REQUIRE_THAT(src, (IsField<Private>("some_int", "int")));
        REQUIRE_THAT(src, (IsField<Private>("some_int_pointer", "int *")));
        REQUIRE_THAT(
            src, (IsField<Private>("some_int_pointer_pointer", "int **")));

        // Relationship members should not be rendered as part of this testcase
        REQUIRE_THAT(src, !(IsField<Private>("a", _A("A"))));
        REQUIRE_THAT(src, !(IsField<Private>("b", _A("B"))));

        REQUIRE_THAT(src, IsAggregation(_A("R"), _A("A"), "-a"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("B"), "-b"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("C"), "-c"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("D"), "-d"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("E"), "-e"));
        REQUIRE_THAT(src, IsAggregation(_A("R"), _A("F"), "-f"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("G"), "-g"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("H"), "-h"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("I"), "-i"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("J"), "-j"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("K"), "-k"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClass(j, "B"));
        REQUIRE(IsClass(j, "C"));
        REQUIRE(IsClass(j, "D"));
        REQUIRE(IsClass(j, "E"));
        REQUIRE(IsClass(j, "F"));
        REQUIRE(IsClass(j, "G"));
        REQUIRE(IsClass(j, "H"));
        REQUIRE(IsClass(j, "I"));
        REQUIRE(IsClass(j, "J"));
        REQUIRE(IsClass(j, "K"));
        REQUIRE(IsClass(j, "R"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);
        mermaid::AliasMatcher _A(src);
        using mermaid::IsField;

        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, IsClass(_A("E")));
        REQUIRE_THAT(src, IsClass(_A("F")));
        REQUIRE_THAT(src, IsClass(_A("G")));
        REQUIRE_THAT(src, IsClass(_A("H")));
        REQUIRE_THAT(src, IsClass(_A("I")));
        REQUIRE_THAT(src, IsClass(_A("J")));
        REQUIRE_THAT(src, IsClass(_A("K")));
        REQUIRE_THAT(src, IsClass(_A("R")));

        REQUIRE_THAT(src, (IsField<Private>("some_int", "int")));
        REQUIRE_THAT(src, (IsField<Private>("some_int_pointer", "int *")));
        REQUIRE_THAT(
            src, (IsField<Private>("some_int_pointer_pointer", "int **")));

        // Relationship members should not be rendered as part of this testcase
        REQUIRE_THAT(src, !(IsField<Private>("a", _A("A"))));
        REQUIRE_THAT(src, !(IsField<Private>("b", _A("B"))));

        REQUIRE_THAT(src, IsAggregation(_A("R"), _A("A"), "-a"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("B"), "-b"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("C"), "-c"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("D"), "-d"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("E"), "-e"));
        REQUIRE_THAT(src, IsAggregation(_A("R"), _A("F"), "-f"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("G"), "-g"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("H"), "-h"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("I"), "-i"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("J"), "-j"));
        REQUIRE_THAT(src, IsAssociation(_A("R"), _A("K"), "-k"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}