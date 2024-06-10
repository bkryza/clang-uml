/**
 * tests/t00002/test_case.h
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

TEST_CASE("t00002")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00002", "t00002_class");

    REQUIRE(diagram->include().namespaces.size() == 1);
    REQUIRE(diagram->exclude().namespaces.size() == 0);

    REQUIRE(model->should_include({"clanguml", "t00002"}, "A"));
    REQUIRE(!model->should_include({"std"}, "vector"));

    CHECK_CLASS_DIAGRAM(
        *config, diagram, *model,
        // Common test case for all diagram types
        [](const auto &src) {
            REQUIRE(HasTitle(src, "Basic class diagram example"));

            REQUIRE(!IsClass(src, "NOSUCHCLASS"));
            REQUIRE(IsAbstractClass(src, "A"));
            REQUIRE(IsClass(src, "B"));
            REQUIRE(IsClass(src, "C"));
            REQUIRE(IsClass(src, "D"));
            REQUIRE(IsBaseClass(src, "A", "B"));
            REQUIRE(IsBaseClass(src, "A", "C"));
            REQUIRE(IsBaseClass(src, "B", "D"));
            REQUIRE(IsBaseClass(src, "C", "D"));

            REQUIRE(IsMethod<Public, Abstract>(src, "A", "foo_a"));
            REQUIRE(IsMethod<Public, Abstract>(src, "C", "foo_c"));

            REQUIRE(IsAssociation<Private>(src, "D", "A", "as"));

            REQUIRE(HasNote(src, "A", "left", "This is class A"));
            REQUIRE(HasNote(src, "B", "top", "This is class B"));

            REQUIRE(HasLink(src, "A",
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t00002/t00002.cc#L7",
                    clanguml::util::get_git_commit()),
                "This is class A"));

            REQUIRE(HasLink(src, "B",
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t00002/t00002.cc#L16",
                    clanguml::util::get_git_commit()),
                "This is class B"));

            REQUIRE(HasMemberLink(src, "+foo_a() : void",
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t00002/t00002.cc#L18",
                    clanguml::util::get_git_commit()),
                "foo_a"));

            REQUIRE(HasMemberLink(src, "-as : std::vector<A *>",
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t00002/t00002.cc#L83",
                    clanguml::util::get_git_commit()),
                "as"));
        },
        // Specific test case only for PlantUML diagram
        [](const plantuml_t &src) {
            REQUIRE(StartsWith(src, "@startuml"s));
            REQUIRE(EndsWith(src, "@enduml\n"s));
        },
        // Specific test case only for JSON diagram
        [](const json_t &src) {
            const auto &A = get_element(src, "A");

            REQUIRE(A.has_value());

            REQUIRE(A.value()["type"] == "class");
            REQUIRE(A.value()["name"] == "A");
            REQUIRE(A.value()["display_name"] == "A");
            REQUIRE(A.value()["namespace"] == "clanguml::t00002");
            REQUIRE(A.value()["source_location"]["file"] == "t00002.cc");
            REQUIRE(A.value()["source_location"]["line"] == 7);
        });
}