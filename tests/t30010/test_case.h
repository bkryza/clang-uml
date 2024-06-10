/**
 * tests/t30010/test_case.h
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

TEST_CASE("t30010")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_PACKAGE_MODEL("t30010", "t30010_package");

    CHECK_PACKAGE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsDirectoryPackage(src, "app"s));
        REQUIRE(IsDirectoryPackage(src, "libraries"s, "lib1"s));
        REQUIRE(IsDirectoryPackage(src, "libraries"s, "lib2"s));
        REQUIRE(IsDirectoryPackage(src, "libraries"s, "lib3"s));
        REQUIRE(IsDirectoryPackage(src, "libraries"s, "lib4"s));

        REQUIRE(IsDependency(src, "app", "lib1"));
        REQUIRE(IsDependency(src, "app", "lib2"));
        REQUIRE(IsDependency(src, "app", "lib3"));
        REQUIRE(IsDependency(src, "app", "lib4"));
    });
}