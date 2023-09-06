/**
 * tests/t00057/test_case.h
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

TEST_CASE("t00057", "[test-case][class]")
{
    auto [config, db] = load_config("t00057");

    auto diagram = config.diagrams["t00057_class"];

    REQUIRE(diagram->name == "t00057_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00057_class");

    {
        auto puml = generate_class_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        // Check if all classes exist
        REQUIRE_THAT(puml, IsClass(_A("t00057_A")));
        REQUIRE_THAT(puml, IsClass(_A("t00057_B")));
        REQUIRE_THAT(puml, IsClass(_A("t00057_C")));
        REQUIRE_THAT(puml, IsUnion(_A("t00057_D")));
        REQUIRE_THAT(puml, IsClass(_A("t00057_E")));
        REQUIRE_THAT(puml, IsClass(_A("t00057_F")));
        REQUIRE_THAT(puml, IsClass(_A("t00057_G")));
        REQUIRE_THAT(puml, !IsClass(_A("(anonymous)")));
        REQUIRE_THAT(puml, IsClass(_A("t00057_R")));

        // Check if all relationships exist
        REQUIRE_THAT(puml, IsAggregation(_A("t00057_R"), _A("t00057_A"), "+a"));
        REQUIRE_THAT(puml, IsAggregation(_A("t00057_R"), _A("t00057_B"), "+b"));
        REQUIRE_THAT(puml, IsAssociation(_A("t00057_R"), _A("t00057_C"), "+c"));
        REQUIRE_THAT(puml, IsAggregation(_A("t00057_R"), _A("t00057_D"), "+d"));
        REQUIRE_THAT(puml, IsAssociation(_A("t00057_R"), _A("t00057_E"), "+e"));
        REQUIRE_THAT(puml, IsAssociation(_A("t00057_R"), _A("t00057_F"), "+f"));
        REQUIRE_THAT(puml,
            IsAggregation(
                _A("t00057_E"), _A("t00057_E::(coordinates)"), "+coordinates"));
        REQUIRE_THAT(puml,
            IsAggregation(_A("t00057_E"), _A("t00057_E::(height)"), "+height"));

        save_puml(config.output_directory(), diagram->name + ".puml", puml);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(get_element(j, "t00057_A").value()["type"] == "class");
        REQUIRE(get_element(j, "t00057_B").value()["type"] == "class");
        REQUIRE(get_element(j, "t00057_C").value()["type"] == "class");
        REQUIRE(get_element(j, "t00057_D").value()["type"] == "class");
        REQUIRE(get_element(j, "t00057_E").value()["type"] == "class");
        REQUIRE(get_element(j, "t00057_F").value()["type"] == "class");
        REQUIRE(get_element(j, "t00057_G").value()["type"] == "class");
        REQUIRE(get_element(j, "t00057_R").value()["type"] == "class");

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto mmd = generate_class_mermaid(diagram, *model);

        save_mermaid(config.output_directory(), diagram->name + ".mmd", mmd);
    }
}