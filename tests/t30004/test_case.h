/**
 * tests/t30004/test_case.h
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

TEST_CASE("t30004")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db] = load_config("t30004");

    auto diagram = config.diagrams["t30004_package"];

    REQUIRE(diagram->name == "t30004_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30004_package");

    CHECK_PACKAGE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsNamespacePackage(src, "A"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AAA"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "BBB"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "CCC"s));
        REQUIRE(!IsNamespacePackage(src, "A"s, "DDD"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "EEE"s));
    });
    /*{
        auto src = generate_package_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsPackage("AAA"));
        REQUIRE_THAT(src, IsPackage("BBB"));
        REQUIRE_THAT(src, IsPackage("CCC"));
        REQUIRE_THAT(src, !IsPackage("DDD"));
        REQUIRE_THAT(src, IsPackage("EEE"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;
        using namespace std::string_literals;

        REQUIRE(IsNamespacePackage(j, "A"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AAA"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "BBB"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "CCC"s));
        REQUIRE(!IsNamespacePackage(j, "A"s, "DDD"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "EEE"s));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_package_mermaid(diagram, *model);
        mermaid::AliasMatcher _A(src);
        using mermaid::IsPackage;

        REQUIRE_THAT(src, IsPackage(_A("AAA")));
        REQUIRE_THAT(src, IsPackage(_A("BBB")));
        REQUIRE_THAT(src, IsPackage(_A("CCC")));
        REQUIRE_THAT(src, !IsPackage(_A("DDD")));
        REQUIRE_THAT(src, IsPackage(_A("EEE")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }*/
}
