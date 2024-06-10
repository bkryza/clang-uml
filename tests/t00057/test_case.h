/**
 * tests/t00057/test_case.h
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

TEST_CASE("t00057")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00057", "t00057_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "t00057_A"));
        REQUIRE(IsClass(src, "t00057_B"));
        REQUIRE(IsClass(src, "t00057_C"));
        REQUIRE(IsUnion(src, "t00057_D"));
        REQUIRE(IsClass(src, "t00057_E"));
        REQUIRE(IsClass(src, "t00057_F"));
        REQUIRE(IsClass(src, "t00057_G"));
        REQUIRE(!IsClass(src, "(anonymous)"));
        REQUIRE(IsClass(src, "t00057_R"));

        REQUIRE(IsAggregation<Public>(src, "t00057_R", "t00057_A", "a"));
        REQUIRE(IsAggregation<Public>(src, "t00057_R", "t00057_B", "b"));
        REQUIRE(IsAssociation<Public>(src, "t00057_R", "t00057_C", "c"));
        REQUIRE(IsAggregation<Public>(src, "t00057_R", "t00057_D", "d"));
        REQUIRE(IsAssociation<Public>(src, "t00057_R", "t00057_E", "e"));
        REQUIRE(IsAssociation<Public>(src, "t00057_R", "t00057_F", "f"));
        REQUIRE(IsAggregation<Public>(
            src, "t00057_E", "t00057_E::(coordinates)", "coordinates"));
        REQUIRE(IsAggregation<Public>(
            src, "t00057_E", "t00057_E::(height)", "height"));
    });
}