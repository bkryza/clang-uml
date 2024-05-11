/**
 * tests/t30009/test_case.h
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

TEST_CASE("t30009")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db] = load_config("t30009");

    auto diagram = config.diagrams["t30009_package"];

    REQUIRE(diagram->name == "t30009_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30009_package");

    CHECK_PACKAGE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsNamespacePackage(src, "One"s));
        REQUIRE(IsNamespacePackage(src, "Two"s));
        REQUIRE(IsNamespacePackage(src, "One"s, "A"s));
        REQUIRE(IsNamespacePackage(src, "One"s, "B"s));
        REQUIRE(IsNamespacePackage(src, "One"s, "C"s));
        REQUIRE(IsNamespacePackage(src, "One"s, "D"s));
        REQUIRE(IsNamespacePackage(src, "Two"s, "A"s));
        REQUIRE(IsNamespacePackage(src, "Two"s, "B"s));
        REQUIRE(IsNamespacePackage(src, "Two"s, "C"s));
        REQUIRE(IsNamespacePackage(src, "Two"s, "D"s));
    });
    /*
        {
            auto src = generate_package_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            // Check if all packages exist
            REQUIRE_THAT(src, IsPackage("One"));
            REQUIRE_THAT(src, IsPackage("Two"));
            REQUIRE_THAT(src, IsPackage("A"));
            REQUIRE_THAT(src, IsPackage("B"));
            REQUIRE_THAT(src, IsPackage("C"));
            REQUIRE_THAT(src, IsPackage("D"));

            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_package_json(diagram, *model);

            using namespace json;
            using namespace std::string_literals;

            REQUIRE(IsNamespacePackage(j, "One"s));
            REQUIRE(IsNamespacePackage(j, "Two"s));
            REQUIRE(IsNamespacePackage(j, "One"s, "A"s));
            REQUIRE(IsNamespacePackage(j, "One"s, "B"s));
            REQUIRE(IsNamespacePackage(j, "One"s, "C"s));
            REQUIRE(IsNamespacePackage(j, "One"s, "D"s));
            REQUIRE(IsNamespacePackage(j, "Two"s, "A"s));
            REQUIRE(IsNamespacePackage(j, "Two"s, "B"s));
            REQUIRE(IsNamespacePackage(j, "Two"s, "C"s));
            REQUIRE(IsNamespacePackage(j, "Two"s, "D"s));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }

        {
            auto src = generate_package_mermaid(diagram, *model);
            mermaid::AliasMatcher _A(src);
            using mermaid::IsPackage;

            REQUIRE_THAT(src, IsPackage(_A("One")));
            REQUIRE_THAT(src, IsPackage(_A("Two")));
            REQUIRE_THAT(src, IsPackage(_A("A")));
            REQUIRE_THAT(src, IsPackage(_A("B")));
            REQUIRE_THAT(src, IsPackage(_A("C")));
            REQUIRE_THAT(src, IsPackage(_A("D")));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }*/
}