/**
 * tests/t30002/test_case.cc
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

TEST_CASE("t30002", "[test-case][package]")
{
    auto [config, db] = load_config("t30002");

    auto diagram = config.diagrams["t30002_package"];

    REQUIRE(diagram->name == "t30002_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30002_package");

    {
        auto puml = generate_package_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        REQUIRE_THAT(puml, IsPackage("A1"));
        REQUIRE_THAT(puml, IsPackage("A2"));
        REQUIRE_THAT(puml, IsPackage("A3"));
        REQUIRE_THAT(puml, IsPackage("A4"));
        REQUIRE_THAT(puml, IsPackage("A5"));
        REQUIRE_THAT(puml, IsPackage("A6"));
        REQUIRE_THAT(puml, IsPackage("A7"));
        REQUIRE_THAT(puml, IsPackage("A8"));
        REQUIRE_THAT(puml, IsPackage("A9"));
        REQUIRE_THAT(puml, IsPackage("A11"));
        REQUIRE_THAT(puml, IsPackage("A12"));
        REQUIRE_THAT(puml, IsPackage("A13"));
        REQUIRE_THAT(puml, IsPackage("A14"));
        REQUIRE_THAT(puml, IsPackage("A15"));
        REQUIRE_THAT(puml, IsPackage("A16"));
        REQUIRE_THAT(puml, IsPackage("A17"));
        REQUIRE_THAT(puml, IsPackage("A18"));

        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A1")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A2")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A3")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A4")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A5")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A6")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A7")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A8")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A9")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A10")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A11")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A12")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A13")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A14")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A15")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A16")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A17")));
        REQUIRE_THAT(puml, IsDependency(_A("BBB"), _A("A18")));

        save_puml(config.output_directory(), diagram->name + ".puml", puml);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;

        REQUIRE(IsPackage(j, "A::AA"));
        REQUIRE(IsPackage(j, "A::AA::A1"));
        REQUIRE(IsPackage(j, "A::AA::A2"));
        REQUIRE(IsPackage(j, "A::AA::A3"));
        REQUIRE(IsPackage(j, "A::AA::A4"));
        REQUIRE(IsPackage(j, "A::AA::A5"));
        REQUIRE(IsPackage(j, "A::AA::A6"));
        REQUIRE(IsPackage(j, "A::AA::A7"));
        REQUIRE(IsPackage(j, "A::AA::A8"));
        REQUIRE(IsPackage(j, "A::AA::A9"));
        REQUIRE(IsPackage(j, "A::AA::A10"));
        REQUIRE(IsPackage(j, "A::AA::A11"));
        REQUIRE(IsPackage(j, "A::AA::A12"));
        REQUIRE(IsPackage(j, "A::AA::A13"));
        REQUIRE(IsPackage(j, "A::AA::A14"));
        REQUIRE(IsPackage(j, "A::AA::A15"));
        REQUIRE(IsPackage(j, "A::AA::A16"));
        REQUIRE(IsPackage(j, "A::AA::A17"));

        REQUIRE(IsPackage(j, "B::BB::BBB"));

        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A1"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A2"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A3"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A4"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A5"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A6"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A7"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A8"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A9"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A10"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A11"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A12"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A13"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A14"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A15"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A16"));
        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::A17"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto mmd = generate_package_mermaid(diagram, *model);

        save_mermaid(config.output_directory(), diagram->name + ".mmd", mmd);
    }
}
