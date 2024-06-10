/**
 * tests/t00054/test_case.h
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

TEST_CASE("t00054")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00054", "t00054_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "a"));
        REQUIRE(IsClass(src, "b"));
        REQUIRE(IsClass(src, {"detail", "c"}));
        REQUIRE(IsClass(src, {"detail", "d"}));
        REQUIRE(IsClass(src, {"detail", "e"}));
        REQUIRE(IsClass(src, "f"));
        REQUIRE(IsClass(src, "g"));

        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, {"detail2", "C"}));
        REQUIRE(IsClass(src, {"detail2::detail3", "D"}));
        REQUIRE(IsClass(src, {"detail2::detail3", "E"}));
        REQUIRE(IsClass(src, {"detail2", "F"}));
        REQUIRE(IsClass(src, "G"));

        REQUIRE(IsEnum(src, {"detail4", "i"}));
        REQUIRE(IsEnum(src, {"detail4", "h"}));
        REQUIRE(IsEnum(src, {"detail4", "j"}));
    });
}