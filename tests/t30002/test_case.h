/**
 * tests/t30002/test_case.h
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

TEST_CASE("t30002")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_PACKAGE_MODEL("t30002", "t30002_package");

    CHECK_PACKAGE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A1"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A2"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A3"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A4"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A5"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A6"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A7"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A8"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A9"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A10"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A11"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A12"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A13"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A14"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A15"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A16"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A17"s));
        REQUIRE(IsNamespacePackage(src, "A"s, "AA"s, "A18"s));

        REQUIRE(IsNamespacePackage(src, "B"s, "BB"s, "BBB"s));

        REQUIRE(IsDependency(src, "BBB", "A1"));
        REQUIRE(IsDependency(src, "BBB", "A2"));
        REQUIRE(IsDependency(src, "BBB", "A3"));
        REQUIRE(IsDependency(src, "BBB", "A4"));
        REQUIRE(IsDependency(src, "BBB", "A5"));
        REQUIRE(IsDependency(src, "BBB", "A6"));
        REQUIRE(IsDependency(src, "BBB", "A7"));
        REQUIRE(IsDependency(src, "BBB", "A8"));
        REQUIRE(IsDependency(src, "BBB", "A9"));
        REQUIRE(IsDependency(src, "BBB", "A10"));
        REQUIRE(IsDependency(src, "BBB", "A11"));
        REQUIRE(IsDependency(src, "BBB", "A12"));
        REQUIRE(IsDependency(src, "BBB", "A13"));
        REQUIRE(IsDependency(src, "BBB", "A14"));
        REQUIRE(IsDependency(src, "BBB", "A15"));
        REQUIRE(IsDependency(src, "BBB", "A16"));
        REQUIRE(IsDependency(src, "BBB", "A17"));
        REQUIRE(IsDependency(src, "BBB", "A18"));
    });
}
