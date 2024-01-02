/**
 * tests/t00043/test_case.cc
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check dependants filter
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("BB")));
        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, IsClass(_A("E")));
        REQUIRE_THAT(src, !IsClass(_A("F")));

        REQUIRE_THAT(src, IsDependency(_A("B"), _A("A")));
        REQUIRE_THAT(src, IsDependency(_A("BB"), _A("A")));
        REQUIRE_THAT(src, IsDependency(_A("C"), _A("B")));
        REQUIRE_THAT(src, IsDependency(_A("D"), _A("C")));
        REQUIRE_THAT(src, IsDependency(_A("E"), _A("D")));

        // Check dependencies filter
        REQUIRE_THAT(src, IsClass(_A("G")));
        REQUIRE_THAT(src, IsClass(_A("GG")));
        REQUIRE_THAT(src, IsClass(_A("H")));
        REQUIRE_THAT(src, !IsClass(_A("HH")));
        REQUIRE_THAT(src, IsClass(_A("I")));
        REQUIRE_THAT(src, IsClass(_A("J")));

        REQUIRE_THAT(src, IsDependency(_A("H"), _A("G")));
        REQUIRE_THAT(src, IsDependency(_A("H"), _A("GG")));
        REQUIRE_THAT(src, IsDependency(_A("I"), _A("H")));
        REQUIRE_THAT(src, IsDependency(_A("J"), _A("I")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
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
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);

        // Check dependants filter
        REQUIRE_THAT(src, IsClass(_A("dependants::A")));
        REQUIRE_THAT(src, IsClass(_A("dependants::B")));
        REQUIRE_THAT(src, IsClass(_A("dependants::BB")));
        REQUIRE_THAT(src, IsClass(_A("dependants::D")));
        REQUIRE_THAT(src, IsClass(_A("dependants::E")));
        REQUIRE_THAT(src, !IsClass(_A("dependants::F")));

        REQUIRE_THAT(
            src, IsDependency(_A("dependants::B"), _A("dependants::A")));
        REQUIRE_THAT(
            src, IsDependency(_A("dependants::BB"), _A("dependants::A")));
        REQUIRE_THAT(
            src, IsDependency(_A("dependants::C"), _A("dependants::B")));
        REQUIRE_THAT(
            src, IsDependency(_A("dependants::D"), _A("dependants::C")));
        REQUIRE_THAT(
            src, IsDependency(_A("dependants::E"), _A("dependants::D")));

        // Check dependencies filter
        REQUIRE_THAT(src, IsClass(_A("dependencies::G")));
        REQUIRE_THAT(src, IsClass(_A("dependencies::GG")));
        REQUIRE_THAT(src, IsClass(_A("dependencies::H")));
        REQUIRE_THAT(src, !IsClass(_A("dependencies::HH")));
        REQUIRE_THAT(src, IsClass(_A("dependencies::I")));
        REQUIRE_THAT(src, IsClass(_A("dependencies::J")));

        REQUIRE_THAT(
            src, IsDependency(_A("dependencies::H"), _A("dependencies::G")));
        REQUIRE_THAT(
            src, IsDependency(_A("dependencies::H"), _A("dependencies::GG")));
        REQUIRE_THAT(
            src, IsDependency(_A("dependencies::I"), _A("dependencies::H")));
        REQUIRE_THAT(
            src, IsDependency(_A("dependencies::J"), _A("dependencies::I")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}