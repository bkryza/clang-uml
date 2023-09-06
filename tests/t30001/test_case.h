/**
 * tests/t30001/test_case.cc
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

TEST_CASE("t30001", "[test-case][package]")
{
    auto [config, db] = load_config("t30001");

    auto diagram = config.diagrams["t30001_package"];

    REQUIRE(diagram->name == "t30001_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30001_package");

    {
        auto puml = generate_package_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        REQUIRE_THAT(puml, IsPackage("A"));
        REQUIRE_THAT(puml, IsPackage("AAA"));
        REQUIRE_THAT(puml, IsPackage("AAA"));

        // TODO: Fix _A() to handle fully qualified names, right
        //       now it only finds the first element with unqualified
        //       name match
        REQUIRE_THAT(puml,
            HasNote(_A("AA"), "top", "This is namespace AA in namespace A"));

        REQUIRE_THAT(puml,
            HasLink(_A("AAA"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t30001/t30001.cc#L6",
                    clanguml::util::get_git_commit()),
                "AAA"));

        REQUIRE_THAT(puml,
            HasLink(_A("BBB"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t30001/t30001.cc#L8",
                    clanguml::util::get_git_commit()),
                "BBB"));

        REQUIRE_THAT(puml, HasComment("t30001 test diagram of type package"));

        save_puml(config.output_directory(), diagram->name + ".puml", puml);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;

        REQUIRE(IsPackage(j, "A"));
        REQUIRE(IsPackage(j, "A::AA"));
        REQUIRE(IsPackage(j, "A::AA::AAA"));
        REQUIRE(IsPackage(j, "A::AA::BBB"));
        REQUIRE(IsPackage(j, "A::BB"));
        REQUIRE(IsPackage(j, "B"));
        REQUIRE(IsPackage(j, "B::AA"));
        REQUIRE(IsPackage(j, "B::AA::AAA"));
        REQUIRE(IsPackage(j, "B::AA::BBB"));
        REQUIRE(IsPackage(j, "B::BB"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
}
