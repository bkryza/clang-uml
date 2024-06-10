/**
 * tests/t40003/test_case.h
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

TEST_CASE("t40003")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_INCLUDE_MODEL("t40003", "t40003_include");

    CHECK_INCLUDE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsFolder(src, "include/dependants"));
        REQUIRE(IsFolder(src, "include/dependencies"));
        REQUIRE(IsFolder(src, "src/dependants"));
        REQUIRE(IsFolder(src, "src/dependencies"));

        REQUIRE(IsFile(src, "include/dependants/t1.h"));
        REQUIRE(IsFile(src, "include/dependants/t2.h"));
        REQUIRE(IsFile(src, "include/dependants/t3.h"));
        REQUIRE(!IsFile(src, "include/dependants/t4.h"));
        REQUIRE(IsFile(src, "src/dependants/t1.cc"));
        REQUIRE(!IsFile(src, "include/dependants/t10.h"));
        REQUIRE(!IsFile(src, "include/dependants/t11.h"));

        REQUIRE(IsFile(src, "include/dependencies/t1.h"));
        REQUIRE(IsFile(src, "include/dependencies/t2.h"));
        REQUIRE(IsFile(src, "include/dependencies/t3.h"));
        REQUIRE(!IsFile(src, "include/dependencies/t4.h"));
        REQUIRE(!IsFile(src, "include/dependencies/t6.h"));

        REQUIRE(IsFile(src, "src/dependencies/t2.cc"));
        REQUIRE(!IsFile(src, "include/dependencies/t7.h"));
        REQUIRE(!IsFile(src, "include/dependencies/t8.h"));
    });
}
