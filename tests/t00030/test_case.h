/**
 * tests/t00030/test_case.h
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

TEST_CASE("t00030")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00030", "t00030_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, "C"));
        REQUIRE(IsClass(src, "D"));

        REQUIRE(IsAssociation<Public>(src, "R", "A", "aaa"));
        REQUIRE(IsComposition<Public>(src, "R", "B", "bbb", "0..1", "1..*"));
        REQUIRE(IsAggregation<Public>(src, "R", "C", "ccc", "0..1", "1..5"));
        REQUIRE(IsAssociation<Public>(src, "R", "D", "ddd", "", "1"));
        REQUIRE(IsAggregation<Public>(src, "R", "E", "eee", "", "1"));
    });
}