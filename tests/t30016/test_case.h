/**
 * tests/t30016/test_case.h
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

TEST_CASE("t30016")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_PACKAGE_MODEL("t30016", "t30016_package");

    CHECK_PACKAGE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsDirectoryPackage(src, "App"s));
        REQUIRE(IsDirectoryPackage(src, "D1"s));
        REQUIRE(IsDirectoryPackage(src, "D2"s));
        REQUIRE(IsDirectoryPackage(src, "D3"s));
        REQUIRE(IsDirectoryPackage(src, "D4"s));
        REQUIRE(IsDirectoryPackage(src, "D5"s));
        REQUIRE(IsDirectoryPackage(src, "D6"s));
        REQUIRE(IsDirectoryPackage(src, "D7"s));
        REQUIRE(IsDirectoryPackage(src, "D8"s));
        REQUIRE(IsDirectoryPackage(src, "D9"s));
        REQUIRE(IsDirectoryPackage(src, "D10"s));
        REQUIRE(IsDirectoryPackage(src, "D11"s));
        REQUIRE(IsDirectoryPackage(src, "D12"s));

        REQUIRE(IsPackageDependency(src, "App"s, "D1"s));
        REQUIRE(IsPackageDependency(src, "App"s, "D2"s));
        REQUIRE(IsPackageDependency(src, "App"s, "D3"s));
        REQUIRE(IsPackageDependency(src, "App"s, "D4"s));
        REQUIRE(IsPackageDependency(src, "App"s, "D5"s));
        REQUIRE(IsPackageDependency(src, "App"s, "D6"s));
        REQUIRE(IsPackageDependency(src, "App"s, "D7"s));
        REQUIRE(IsPackageDependency(src, "App"s, "D11"s));
        REQUIRE(IsPackageDependency(src, "App"s, "D12"s));

        REQUIRE(IsPackageDependency(src, "D7"s, "D8"s));
        REQUIRE(IsPackageDependency(src, "D7"s, "D9"s));

        REQUIRE(IsPackageDependency(src, "D10"s, "D5"s));
        REQUIRE(IsPackageDependency(src, "D10"s, "D8"s));
    });
}