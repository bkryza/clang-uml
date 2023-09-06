/**
 * tests/t00043/test_case.cc
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

TEST_CASE("t00043", "[test-case][class]")
{
    auto [config, db] = load_config("t00043");

    auto diagram = config.diagrams["t00043_class"];

    REQUIRE(diagram->name == "t00043_class");
    REQUIRE(diagram->generate_packages() == true);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00043_class");

    {
        auto puml = generate_class_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        // Check dependants filter
        REQUIRE_THAT(puml, IsClass(_A("A")));
        REQUIRE_THAT(puml, IsClass(_A("B")));
        REQUIRE_THAT(puml, IsClass(_A("BB")));
        REQUIRE_THAT(puml, IsClass(_A("D")));
        REQUIRE_THAT(puml, IsClass(_A("E")));
        REQUIRE_THAT(puml, !IsClass(_A("F")));

        REQUIRE_THAT(puml, IsDependency(_A("B"), _A("A")));
        REQUIRE_THAT(puml, IsDependency(_A("BB"), _A("A")));
        REQUIRE_THAT(puml, IsDependency(_A("C"), _A("B")));
        REQUIRE_THAT(puml, IsDependency(_A("D"), _A("C")));
        REQUIRE_THAT(puml, IsDependency(_A("E"), _A("D")));

        // Check dependencies filter
        REQUIRE_THAT(puml, IsClass(_A("G")));
        REQUIRE_THAT(puml, IsClass(_A("GG")));
        REQUIRE_THAT(puml, IsClass(_A("H")));
        REQUIRE_THAT(puml, !IsClass(_A("HH")));
        REQUIRE_THAT(puml, IsClass(_A("I")));
        REQUIRE_THAT(puml, IsClass(_A("J")));

        REQUIRE_THAT(puml, IsDependency(_A("H"), _A("G")));
        REQUIRE_THAT(puml, IsDependency(_A("H"), _A("GG")));
        REQUIRE_THAT(puml, IsDependency(_A("I"), _A("H")));
        REQUIRE_THAT(puml, IsDependency(_A("J"), _A("I")));

        save_puml(config.output_directory(), diagram->name + ".puml", puml);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "dependants::A"));
        REQUIRE(IsClass(j, "dependants::B"));
        REQUIRE(IsClass(j, "dependants::C"));
        REQUIRE(IsClass(j, "dependants::D"));
        REQUIRE(IsClass(j, "dependants::BB"));
        REQUIRE(IsClass(j, "dependants::E"));
        REQUIRE(IsDependency(j, "dependants::B", "dependants::A"));

        REQUIRE(IsClass(j, "dependencies::G"));
        REQUIRE(IsClass(j, "dependencies::GG"));
        REQUIRE(IsClass(j, "dependencies::H"));
        REQUIRE(IsDependency(j, "dependencies::J", "dependencies::I"));
        REQUIRE(IsDependency(j, "dependencies::H", "dependencies::G"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto mmd = generate_class_mermaid(diagram, *model);

        save_mermaid(config.output_directory(), diagram->name + ".mmd", mmd);
    }
}