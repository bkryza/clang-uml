/**
 * tests/t00002/test_case.cc
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

#include "../test_cases.h"

TEST_CASE("t00002")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00002");

    auto diagram = config.diagrams["t00002_class"];

    REQUIRE(diagram->name == "t00002_class");

    REQUIRE(diagram->include().namespaces.size() == 1);

    REQUIRE(diagram->exclude().namespaces.size() == 0);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00002_class");

    REQUIRE(model->should_include({"clanguml", "t00002"}, "A"));
    REQUIRE(!model->should_include({"std"}, "vector"));

    CHECK_CLASS_DIAGRAM(
        diagram, *model,
        // Common test case for all diagram types
        [](const auto &src) {
            REQUIRE(!IsClass(src, "NOSUCHCLASS"));
            REQUIRE(IsClass(src, "A"));
        },
        // Specific test case only for JSON diagram
        [](const json_t &src) {
            const auto &A = get_element(src, "A");

            CHECK(A.has_value());

            CHECK(A.value()["type"] == "class");
            CHECK(A.value()["name"] == "A");
            CHECK(A.value()["display_name"] == "A");
            CHECK(A.value()["namespace"] == "clanguml::t00002");
            CHECK(A.value()["source_location"]["file"] == "t00002.cc");
            CHECK(A.value()["source_location"]["line"] == 7);
        });

    // auto src = render_class_diagram<plantuml_t>(diagram, *model);

    /*


        {
            auto puml = generate_class_puml(diagram, *model);
            AliasMatcher _A(puml);

            REQUIRE_THAT(puml, StartsWith("@startuml"));
            REQUIRE_THAT(puml, EndsWith("@enduml\n"));
            REQUIRE_THAT(puml, HasTitle("Basic class diagram example"));
            REQUIRE_THAT(puml, IsAbstractClass(_A("A")));
            REQUIRE_THAT(puml, IsClass(_A("B")));
            REQUIRE_THAT(puml, IsClass(_A("C")));
            REQUIRE_THAT(puml, IsClass(_A("D")));
            REQUIRE_THAT(puml, IsBaseClass(_A("A"), _A("B")));
            REQUIRE_THAT(puml, IsBaseClass(_A("A"), _A("C")));
            REQUIRE_THAT(puml, IsBaseClass(_A("B"), _A("D")));
            REQUIRE_THAT(puml, IsBaseClass(_A("C"), _A("D")));
            REQUIRE_THAT(puml, (IsMethod<Public, Abstract>("foo_a")));
            REQUIRE_THAT(puml, (IsMethod<Public, Abstract>("foo_c")));

            REQUIRE_THAT(puml, IsAssociation(_A("D"), _A("A"), "-as"));

            REQUIRE_THAT(puml, HasNote(_A("A"), "left", "This is class A"));
            REQUIRE_THAT(puml, HasNote(_A("B"), "top", "This is class B"));

            REQUIRE_THAT(puml,
                HasLink(_A("A"),
                    fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                                "t00002/t00002.cc#L7",
                        clanguml::util::get_git_commit()),
                    "This is class A"));

            REQUIRE_THAT(puml,
                HasLink(_A("B"),
                    fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                                "t00002/t00002.cc#L16",
                        clanguml::util::get_git_commit()),
                    "This is class B"));

            REQUIRE_THAT(puml,
                HasMemberLink("+foo_a() : void",
                    fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                                "t00002/t00002.cc#L18",
                        clanguml::util::get_git_commit()),
                    "foo_a"));

            REQUIRE_THAT(puml,
                HasMemberLink("-as : std::vector<A *>",
                    fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                                "t00002/t00002.cc#L83",
                        clanguml::util::get_git_commit()),
                    "as"));

            save_puml(config.output_directory(), diagram->name + ".puml", puml);
        }
        {
            auto j = generate_class_json(diagram, *model);

            using namespace json;

            const auto &A = get_element(j, "A");

            CHECK(A.has_value());

            CHECK(A.value()["type"] == "class");
            CHECK(A.value()["name"] == "A");
            CHECK(A.value()["display_name"] == "A");
            CHECK(A.value()["namespace"] == "clanguml::t00002");
            CHECK(A.value()["source_location"]["file"] == "t00002.cc");
            CHECK(A.value()["source_location"]["line"] == 7);

            REQUIRE(HasTitle(j, "Basic class diagram example"));
            REQUIRE(IsClass(j, "A"));
            REQUIRE(IsClass(j, "B"));
            REQUIRE(IsClass(j, "C"));
            REQUIRE(IsBaseClass(j, "A", "B"));
            REQUIRE(IsBaseClass(j, "A", "C"));
            REQUIRE(IsBaseClass(j, "B", "D"));
            REQUIRE(IsBaseClass(j, "C", "D"));
            REQUIRE(IsMethod(j, "A", "foo_a"));
            REQUIRE(IsMethod(j, "C", "foo_c"));
            REQUIRE(IsField(j, "E", "as", "std::vector<A *>"));
            REQUIRE(IsAssociation(j, "D", "A", "as"));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }
        {
            auto mmd = generate_class_mermaid(diagram, *model);

            mermaid::AliasMatcher _A(mmd);
            using mermaid::HasNote;
            using mermaid::HasTitle;
            using mermaid::IsAbstractClass;

            REQUIRE_THAT(mmd, HasTitle("Basic class diagram example"));
            REQUIRE_THAT(mmd, Contains("classDiagram"));
            REQUIRE_THAT(mmd, IsAbstractClass(_A("A")));
            REQUIRE_THAT(mmd, IsClass(_A("B")));
            REQUIRE_THAT(mmd, IsClass(_A("C")));
            REQUIRE_THAT(mmd, IsClass(_A("D")));
            REQUIRE_THAT(mmd, IsBaseClass(_A("A"), _A("B")));
            REQUIRE_THAT(mmd, IsBaseClass(_A("A"), _A("C")));
            REQUIRE_THAT(mmd, IsBaseClass(_A("B"), _A("D")));
            REQUIRE_THAT(mmd, IsBaseClass(_A("C"), _A("D")));

            REQUIRE_THAT(mmd, IsAssociation(_A("D"), _A("A"), "-as"));

            REQUIRE_THAT(mmd, (mermaid::IsMethod<Public, Abstract>("foo_a")));
            REQUIRE_THAT(mmd, (mermaid::IsMethod<Public, Abstract>("foo_c")));

            REQUIRE_THAT(mmd, HasNote(_A("A"), "left", "This is class A"));
            REQUIRE_THAT(mmd, HasNote(_A("B"), "top", "This is class B"));

            REQUIRE_THAT(mmd,
                mermaid::HasLink(_A("A"),
                    fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                                "t00002/t00002.cc#L7",
                        clanguml::util::get_git_commit()),
                    "This is class A"));

            REQUIRE_THAT(mmd,
                mermaid::HasLink(_A("B"),
                    fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                                "t00002/t00002.cc#L16",
                        clanguml::util::get_git_commit()),
                    "This is class B"));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       mmd);
        }
        */
}