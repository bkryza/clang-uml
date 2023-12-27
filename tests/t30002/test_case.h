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
        auto src = generate_package_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsPackage("A1"));
        REQUIRE_THAT(src, IsPackage("A2"));
        REQUIRE_THAT(src, IsPackage("A3"));
        REQUIRE_THAT(src, IsPackage("A4"));
        REQUIRE_THAT(src, IsPackage("A5"));
        REQUIRE_THAT(src, IsPackage("A6"));
        REQUIRE_THAT(src, IsPackage("A7"));
        REQUIRE_THAT(src, IsPackage("A8"));
        REQUIRE_THAT(src, IsPackage("A9"));
        REQUIRE_THAT(src, IsPackage("A11"));
        REQUIRE_THAT(src, IsPackage("A12"));
        REQUIRE_THAT(src, IsPackage("A13"));
        REQUIRE_THAT(src, IsPackage("A14"));
        REQUIRE_THAT(src, IsPackage("A15"));
        REQUIRE_THAT(src, IsPackage("A16"));
        REQUIRE_THAT(src, IsPackage("A17"));
        REQUIRE_THAT(src, IsPackage("A18"));

        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A1")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A2")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A3")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A4")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A5")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A6")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A7")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A8")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A9")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A10")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A11")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A12")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A13")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A14")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A15")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A16")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A17")));
        REQUIRE_THAT(src, IsDependency(_A("BBB"), _A("A18")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;
        using namespace std::string_literals;

        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A1"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A2"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A3"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A4"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A5"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A6"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A7"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A8"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A9"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A10"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A11"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A12"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A13"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A14"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A15"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A16"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A17"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "A18"s));

        REQUIRE(IsNamespacePackage(j, "B"s, "BB"s, "BBB"s));

        REQUIRE(IsDependency(j, "BBB", "A1"));
        REQUIRE(IsDependency(j, "BBB", "A2"));
        REQUIRE(IsDependency(j, "BBB", "A3"));
        REQUIRE(IsDependency(j, "BBB", "A4"));
        REQUIRE(IsDependency(j, "BBB", "A5"));
        REQUIRE(IsDependency(j, "BBB", "A6"));
        REQUIRE(IsDependency(j, "BBB", "A7"));
        REQUIRE(IsDependency(j, "BBB", "A8"));
        REQUIRE(IsDependency(j, "BBB", "A9"));
        REQUIRE(IsDependency(j, "BBB", "A10"));
        REQUIRE(IsDependency(j, "BBB", "A11"));
        REQUIRE(IsDependency(j, "BBB", "A12"));
        REQUIRE(IsDependency(j, "BBB", "A13"));
        REQUIRE(IsDependency(j, "BBB", "A14"));
        REQUIRE(IsDependency(j, "BBB", "A15"));
        REQUIRE(IsDependency(j, "BBB", "A16"));
        REQUIRE(IsDependency(j, "BBB", "A17"));
        REQUIRE(IsDependency(j, "BBB", "A18"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_package_mermaid(diagram, *model);
        mermaid::AliasMatcher _A(src);
        using mermaid::IsPackage;
        using mermaid::IsPackageDependency;

        REQUIRE_THAT(src, IsPackage(_A("A1")));
        REQUIRE_THAT(src, IsPackage(_A("A2")));
        REQUIRE_THAT(src, IsPackage(_A("A3")));
        REQUIRE_THAT(src, IsPackage(_A("A4")));
        REQUIRE_THAT(src, IsPackage(_A("A5")));
        REQUIRE_THAT(src, IsPackage(_A("A6")));
        REQUIRE_THAT(src, IsPackage(_A("A7")));
        REQUIRE_THAT(src, IsPackage(_A("A8")));
        REQUIRE_THAT(src, IsPackage(_A("A9")));
        REQUIRE_THAT(src, IsPackage(_A("A11")));
        REQUIRE_THAT(src, IsPackage(_A("A12")));
        REQUIRE_THAT(src, IsPackage(_A("A13")));
        REQUIRE_THAT(src, IsPackage(_A("A14")));
        REQUIRE_THAT(src, IsPackage(_A("A15")));
        REQUIRE_THAT(src, IsPackage(_A("A16")));
        REQUIRE_THAT(src, IsPackage(_A("A17")));
        REQUIRE_THAT(src, IsPackage(_A("A18")));

        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A1")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A2")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A3")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A4")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A5")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A6")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A7")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A8")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A9")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A10")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A11")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A12")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A13")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A14")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A15")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A16")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A17")));
        REQUIRE_THAT(src, IsPackageDependency(_A("BBB"), _A("A18")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}
