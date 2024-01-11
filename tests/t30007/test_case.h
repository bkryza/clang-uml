/**
 * tests/t30007/test_case.cc
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

TEST_CASE("t30007", "[test-case][package]")
{
    auto [config, db] = load_config("t30007");

    auto diagram = config.diagrams["t30007_package"];

    REQUIRE(diagram->name == "t30007_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30007_package");

    {
        auto src = generate_package_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsPackage("A"));
        REQUIRE_THAT(src, IsPackage("B"));
        REQUIRE_THAT(src, IsPackage("C"));

        REQUIRE_THAT(src, IsDependency(_A("AA"), _A("B")));
        REQUIRE_THAT(src, IsDependency(_A("AA"), _A("C")));

        REQUIRE_THAT(src, IsLayoutHint(_A("C"), "up", _A("AA")));
        REQUIRE_THAT(src, IsLayoutHint(_A("C"), "left", _A("B")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;
        using namespace std::string_literals;

        REQUIRE(IsNamespacePackage(j, "A"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s));
        REQUIRE(IsNamespacePackage(j, "B"s));
        REQUIRE(IsNamespacePackage(j, "C"s));

        REQUIRE(IsDependency(j, "AA", "B"));
        REQUIRE(IsDependency(j, "AA", "C"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_package_mermaid(diagram, *model);
        mermaid::AliasMatcher _A(src);
        using mermaid::IsPackage;
        using mermaid::IsPackageDependency;

        REQUIRE_THAT(src, IsPackage(_A("A")));
        REQUIRE_THAT(src, IsPackage(_A("B")));
        REQUIRE_THAT(src, IsPackage(_A("C")));

        REQUIRE_THAT(src, IsPackageDependency(_A("AA"), _A("B")));
        REQUIRE_THAT(src, IsPackageDependency(_A("AA"), _A("C")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}
