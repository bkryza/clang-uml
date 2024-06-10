/**
 * tests/t00035/test_case.h
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

TEST_CASE("t00035")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00035", "t00035_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "Top"));
        REQUIRE(IsClass(src, "Bottom"));
        REQUIRE(IsClass(src, "Center"));
        REQUIRE(IsClass(src, "Left"));
        REQUIRE(IsClass(src, "Right"));

        REQUIRE(IsLayoutHint(src, "Center", "up", "Top"));
        REQUIRE(IsLayoutHint(src, "Center", "left", "Left"));
        REQUIRE(IsLayoutHint(src, "Center", "right", "Right"));
        REQUIRE(IsLayoutHint(src, "Center", "down", "Bottom"));
    });
}