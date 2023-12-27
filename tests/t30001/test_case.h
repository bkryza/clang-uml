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
        auto src = generate_package_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, HasTitle("Basic package diagram example"));

        REQUIRE_THAT(src, IsPackage("A"));
        REQUIRE_THAT(src, IsPackage("AAA"));
        REQUIRE_THAT(src, IsPackage("AAA"));

        // TODO: Fix _A() to handle fully qualified names, right
        //       now it only finds the first element with unqualified
        //       name match
        REQUIRE_THAT(src,
            HasNote(_A("AA"), "top", "This is namespace AA in namespace A"));

        REQUIRE_THAT(src,
            HasLink(_A("AAA"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t30001/t30001.cc#L6",
                    clanguml::util::get_git_commit()),
                "AAA"));

        REQUIRE_THAT(src,
            HasLink(_A("BBB"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t30001/t30001.cc#L8",
                    clanguml::util::get_git_commit()),
                "BBB"));

        REQUIRE_THAT(src, HasComment("t30001 test diagram of type package"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;
        using namespace std::string_literals;

        REQUIRE(HasTitle(j, "Basic package diagram example"));

        REQUIRE(!IsNamespacePackage(j, "clanguml"s));
        REQUIRE(!IsNamespacePackage(j, "t30001"s));
        REQUIRE(IsNamespacePackage(j, "A"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s));
        REQUIRE(IsNamespacePackage(j, "A"s, "AA"s, "AAA"s));
        REQUIRE(IsNamespacePackage(j, "B"s, "AA"s, "AAA"s));
        REQUIRE(IsNamespacePackage(j, "B"s, "AA"s, "BBB"s));
        REQUIRE(IsNamespacePackage(j, "B"s, "BB"s));
        REQUIRE(IsNamespacePackage(j, "B"s));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_package_mermaid(diagram, *model);
        mermaid::AliasMatcher _A(src);

        using mermaid::HasComment;
        using mermaid::HasLink;
        using mermaid::HasPackageNote;
        using mermaid::HasTitle;
        using mermaid::IsPackage;

        REQUIRE_THAT(src, HasTitle("Basic package diagram example"));

        REQUIRE_THAT(src, IsPackage(_A("A")));
        REQUIRE_THAT(src, IsPackage(_A("AAA")));
        REQUIRE_THAT(src, IsPackage(_A("AAA")));

        // TODO: Fix _A() to handle fully qualified names, right
        //       now it only finds the first element with unqualified
        //       name match
        REQUIRE_THAT(src,
            HasPackageNote(
                _A("AA"), "top", "This is namespace AA in namespace A"));

        REQUIRE_THAT(src,
            HasLink(_A("AAA"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t30001/t30001.cc#L6",
                    clanguml::util::get_git_commit()),
                "AAA"));

        REQUIRE_THAT(src,
            HasLink(_A("BBB"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t30001/t30001.cc#L8",
                    clanguml::util::get_git_commit()),
                "BBB"));

        REQUIRE_THAT(src, HasComment("t30001 test diagram of type package"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}
