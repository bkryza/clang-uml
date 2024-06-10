/**
 * tests/t30001/test_case.h
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

TEST_CASE("t30001")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_PACKAGE_MODEL("t30001", "t30001_package");

    CHECK_PACKAGE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(HasTitle(src, "Basic package diagram example"));

        REQUIRE(!IsNamespacePackage(src, "clanguml"s));
        REQUIRE(!IsNamespacePackage(src, "t30001"s));
        REQUIRE(IsNamespacePackage(src, "A"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "AAA"s));
        REQUIRE(IsNamespacePackage(src, "B"s, "AA"s, "AAA"s));
        REQUIRE(IsNamespacePackage(src, "B"s, "AA"s, "BBB"s));
        REQUIRE(IsNamespacePackage(src, "B"s, "BB"s));
        REQUIRE(IsNamespacePackage(src, "B"s));

        REQUIRE(
            HasNote(src, "AA", "top", "This is namespace AA in namespace A"));

        REQUIRE(HasLink(src, "AAA",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t30001/t30001.cc#L6",
                clanguml::util::get_git_commit()),
            "AAA"));

        REQUIRE(HasLink(src, "BBB",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t30001/t30001.cc#L8",
                clanguml::util::get_git_commit()),
            "BBB"));

        REQUIRE(HasComment(src, "t30001 test diagram of type package"));
    });
}
