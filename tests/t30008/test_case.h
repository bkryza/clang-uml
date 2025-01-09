/**
 * tests/t30008/test_case.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t30008")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_PACKAGE_MODEL("t30008", "t30008_package");

    CHECK_PACKAGE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsNamespacePackage(src, "dependants"s, "A"s));
        REQUIRE(IsNamespacePackage(src, "dependants"s, "B"s));
        REQUIRE(IsNamespacePackage(src, "dependants"s, "C"s));
        REQUIRE(!IsNamespacePackage(src, "dependants"s, "X"s));

        REQUIRE(IsDependency(src, "B", "A"));
        REQUIRE(IsDependency(src, "C", "B"));

        REQUIRE(IsNamespacePackage(src, "dependencies"s, "D"s));
        REQUIRE(IsNamespacePackage(src, "dependencies"s, "E"s));
        REQUIRE(IsNamespacePackage(src, "dependencies"s, "F"s));
        REQUIRE(!IsNamespacePackage(src, "dependencies"s, "Y"s));

        REQUIRE(IsDependency(src, "E", "D"));
        REQUIRE(IsDependency(src, "F", "E"));
    });
}
