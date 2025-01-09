/**
 * tests/t40004/test_case.h
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

TEST_CASE("t40004")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_INCLUDE_MODEL("t40004", "t40004_include");

    CHECK_INCLUDE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsFolder(src, "src"));
        REQUIRE(IsFolder(src, "src/lib1"));
        REQUIRE(IsFolder(src, "src/lib2"));
        REQUIRE(IsFolder(src, "include"));
        REQUIRE(IsFolder(src, "include/lib1"));
        REQUIRE(IsFolder(src, "include/lib2"));

        REQUIRE(IsFile(src, "include/lib1/lib1.h"));
        REQUIRE(IsFile(src, "include/lib2/lib2.h"));

        REQUIRE(IsFile(src, "src/t40004.m"));

        REQUIRE(IsFile(src, "src/lib1/lib1.m"));
        REQUIRE(IsFile(src, "src/lib2/lib2.m"));

        REQUIRE(IsHeaderDependency(src, "src/t40004.m", "include/lib1/lib1.h"));
        REQUIRE(IsHeaderDependency(src, "src/t40004.m", "include/lib2/lib2.h"));
        REQUIRE(
            IsHeaderDependency(src, "src/lib1/lib1.m", "include/lib1/lib1.h"));
        REQUIRE(
            IsHeaderDependency(src, "src/lib2/lib2.m", "include/lib2/lib2.h"));
    });
}