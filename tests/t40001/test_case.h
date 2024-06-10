/**
 * tests/t40001/test_case.h
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

TEST_CASE("t40001")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_INCLUDE_MODEL("t40001", "t40001_include");

    CHECK_INCLUDE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(HasTitle(src, "Basic include diagram example"));

        REQUIRE(IsFolder(src, "include/lib1"));
        REQUIRE(IsFile(src, "include/lib1/lib1.h"));
        REQUIRE(IsFile(src, "src/t40001.cc"));
        REQUIRE(IsFile(src, "include/t40001_include1.h"));

        REQUIRE(IsSystemHeader(src, "string"));
        REQUIRE(IsSystemHeader(src, "yaml-cpp/yaml.h"));

        REQUIRE(IsHeaderDependency(
            src, "src/t40001.cc", "include/t40001_include1.h"));
        REQUIRE(IsHeaderDependency(
            src, "include/t40001_include1.h", "include/lib1/lib1.h"));

        REQUIRE(IsSystemHeaderDependency(
            src, "include/t40001_include1.h", "string"));
    });
}
