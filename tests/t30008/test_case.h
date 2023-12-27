/**
 * tests/t30008/test_case.cc
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

TEST_CASE("t30008", "[test-case][package]")
{
    auto [config, db] = load_config("t30008");

    auto diagram = config.diagrams["t30008_package"];

    REQUIRE(diagram->name == "t30008_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30008_package");

    {
        auto src = generate_package_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsPackage("A"));
        REQUIRE_THAT(src, IsPackage("B"));
        REQUIRE_THAT(src, IsPackage("C"));
        REQUIRE_THAT(src, !IsPackage("X"));

        REQUIRE_THAT(src, IsDependency(_A("B"), _A("A")));
        REQUIRE_THAT(src, IsDependency(_A("C"), _A("B")));

        REQUIRE_THAT(src, IsPackage("D"));
        REQUIRE_THAT(src, IsPackage("E"));
        REQUIRE_THAT(src, IsPackage("F"));
        REQUIRE_THAT(src, !IsPackage("Y"));

        REQUIRE_THAT(src, IsDependency(_A("E"), _A("D")));
        REQUIRE_THAT(src, IsDependency(_A("F"), _A("E")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;
        using namespace std::string_literals;

        REQUIRE(IsNamespacePackage(j, "dependants"s, "A"s));
        REQUIRE(IsNamespacePackage(j, "dependants"s, "B"s));
        REQUIRE(IsNamespacePackage(j, "dependants"s, "C"s));
        REQUIRE(!IsNamespacePackage(j, "dependants"s, "X"s));

        REQUIRE(IsDependency(j, "B", "A"));
        REQUIRE(IsDependency(j, "C", "B"));

        REQUIRE(IsNamespacePackage(j, "dependencies"s, "D"s));
        REQUIRE(IsNamespacePackage(j, "dependencies"s, "E"s));
        REQUIRE(IsNamespacePackage(j, "dependencies"s, "F"s));
        REQUIRE(!IsNamespacePackage(j, "dependencies"s, "Y"s));

        REQUIRE(IsDependency(j, "E", "D"));
        REQUIRE(IsDependency(j, "F", "E"));

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
        REQUIRE_THAT(src, !IsPackage(_A("X")));

        REQUIRE_THAT(src, IsPackageDependency(_A("B"), _A("A")));
        REQUIRE_THAT(src, IsPackageDependency(_A("C"), _A("B")));

        REQUIRE_THAT(src, IsPackage(_A("D")));
        REQUIRE_THAT(src, IsPackage(_A("E")));
        REQUIRE_THAT(src, IsPackage(_A("F")));
        REQUIRE_THAT(src, !IsPackage(_A("Y")));

        REQUIRE_THAT(src, IsPackageDependency(_A("E"), _A("D")));
        REQUIRE_THAT(src, IsPackageDependency(_A("F"), _A("E")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}
