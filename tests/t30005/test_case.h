/**
 * tests/t30005/test_case.cc
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

TEST_CASE("t30005", "[test-case][package]")
{
    auto [config, db] = load_config("t30005");

    auto diagram = config.diagrams["t30005_package"];

    REQUIRE(diagram->name == "t30005_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30005_package");

    {
        auto src = generate_package_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsPackage("AAA"));
        REQUIRE_THAT(src, IsPackage("BBB"));
        REQUIRE_THAT(src, IsPackage("CCC"));

        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("AAA")));
        REQUIRE_THAT(src, IsDependency(_A("CCC"), _A("AAA")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;

        REQUIRE(IsPackage(j, "A"));
        REQUIRE(IsPackage(j, "A::AA"));
        REQUIRE(IsPackage(j, "A::AA::AAA"));
        REQUIRE(IsPackage(j, "B"));
        REQUIRE(IsPackage(j, "B::BB"));
        REQUIRE(IsPackage(j, "B::BB::BBB"));
        REQUIRE(IsPackage(j, "C"));
        REQUIRE(IsPackage(j, "C::CC"));
        REQUIRE(IsPackage(j, "C::CC::CCC"));

        REQUIRE(IsDependency(j, "B::BB::BBB", "A::AA::AAA"));
        REQUIRE(IsDependency(j, "C::CC::CCC", "A::AA::AAA"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_package_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsPackage;
        using mermaid::IsPackageDependency;

        REQUIRE_THAT(src, IsPackage(_A("AAA")));
        REQUIRE_THAT(src, IsPackage(_A("BBB")));
        REQUIRE_THAT(src, IsPackage(_A("CCC")));

        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("AAA")));
        REQUIRE_THAT(src, IsPackageDependency(_A("CCC"), _A("AAA")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}
