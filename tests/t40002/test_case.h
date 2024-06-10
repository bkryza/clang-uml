/**
 * tests/t40002/test_case.h
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

TEST_CASE("t40002")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_INCLUDE_MODEL("t40002", "t40002_include");

    CHECK_INCLUDE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsFolder(src, "include"));
        REQUIRE(IsFolder(src, "include/lib1"));
        REQUIRE(IsFolder(src, "include/lib2"));
        REQUIRE(IsFolder(src, "src"));
        REQUIRE(IsFolder(src, "src/lib1"));
        REQUIRE(IsFolder(src, "src/lib2"));
        REQUIRE(IsFile(src, "include/lib1/lib1.h"));
        REQUIRE(IsFile(src, "include/lib2/lib2.h"));
        REQUIRE(!IsFile(src, "include/lib2/lib2_detail.h"));
        REQUIRE(IsFile(src, "src/lib1/lib1.cc"));
        REQUIRE(IsFile(src, "src/lib2/lib2.cc"));
        REQUIRE(IsFile(src, "src/t40002.cc"));

        REQUIRE(!IsFile(src, "string"));

        REQUIRE(
            IsHeaderDependency(src, "src/t40002.cc", "include/lib1/lib1.h"));
        REQUIRE(IsHeaderDependency(
            src, "include/lib1/lib1.h", "include/lib2/lib2.h"));
        REQUIRE(
            IsHeaderDependency(src, "src/lib1/lib1.cc", "include/lib1/lib1.h"));
        REQUIRE(
            IsHeaderDependency(src, "src/lib2/lib2.cc", "include/lib2/lib2.h"));

        REQUIRE(HasLink(src, "t40002.cc",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t40002/src/t40002.cc#L0",
                clanguml::util::get_git_commit()),
            "t40002.cc"));

        REQUIRE(HasLink(src, "lib1.cc",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t40002/src/lib1/lib1.cc#L0",
                clanguml::util::get_git_commit()),
            "lib1.cc"));

        REQUIRE(HasLink(src, "lib1.h",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t40002/include/lib1/lib1.h#L0",
                clanguml::util::get_git_commit()),
            "lib1.h"));

        REQUIRE(HasLink(src, "lib2.h",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t40002/include/lib2/lib2.h#L0",
                clanguml::util::get_git_commit()),
            "lib2.h"));
    });
}
