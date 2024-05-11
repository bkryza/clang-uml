/**
 * tests/t30005/test_case.h
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

TEST_CASE("t30005")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db] = load_config("t30005");

    auto diagram = config.diagrams["t30005_package"];

    REQUIRE(diagram->name == "t30005_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30005_package");

    CHECK_PACKAGE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsNamespacePackage(src, "A"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "AAA"s));
        REQUIRE(IsNamespacePackage(src, "B"s));
        REQUIRE(IsNamespacePackage(src, "B"s, "BB"s));
        REQUIRE(IsNamespacePackage(src, "B"s, "BB"s, "BBB"s));
        REQUIRE(IsNamespacePackage(src, "C"s));
        REQUIRE(IsNamespacePackage(src, "C"s, "CC"s));
        REQUIRE(IsNamespacePackage(src, "C"s, "CC"s, "CCC"s));

        REQUIRE(IsDependency(src, "BBB", "AAA"));
        REQUIRE(IsDependency(src, "CCC", "AAA"));
    });
/*
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
        using namespace std::string_literals;

        REQUIRE(IsNamespacePackage(j, "A"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "AAA"s));
        REQUIRE(IsNamespacePackage(j, "B"s));
        REQUIRE(IsNamespacePackage(j, "B"s, "BB"s));
        REQUIRE(IsNamespacePackage(j, "B"s, "BB"s, "BBB"s));
        REQUIRE(IsNamespacePackage(j, "C"s));
        REQUIRE(IsNamespacePackage(j, "C"s, "CC"s));
        REQUIRE(IsNamespacePackage(j, "C"s, "CC"s, "CCC"s));

        REQUIRE(IsDependency(j, "BBB", "AAA"));
        REQUIRE(IsDependency(j, "CCC", "AAA"));

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
    }*/
}
