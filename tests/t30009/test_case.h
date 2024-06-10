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

    auto [config, db, diagram, model] =
        CHECK_PACKAGE_MODEL("t30009", "t30009_package");

    CHECK_PACKAGE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
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
}