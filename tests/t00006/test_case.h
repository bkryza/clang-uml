/**
 * tests/t00006/test_case.h
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

TEST_CASE("t00006")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00006", "t00006_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, "C"));
        REQUIRE(IsClass(src, "D"));
        REQUIRE(IsClass(src, "E"));
        REQUIRE(IsClass(src, "F"));
        REQUIRE(IsClass(src, "G"));
        REQUIRE(IsClass(src, "H"));
        REQUIRE(IsClass(src, "I"));
        REQUIRE(IsClass(src, "J"));
        REQUIRE(IsClass(src, "K"));
        REQUIRE(IsClass(src, "L"));
        REQUIRE(IsClass(src, "M"));
        REQUIRE(IsClass(src, "N"));
        REQUIRE(IsClass(src, "NN"));
        REQUIRE(IsClass(src, "NNN"));

        REQUIRE(
            IsInstantiation(src, "custom_container<T>", "custom_container<E>"));

        REQUIRE(IsAggregation<Public>(src, "R", "A", "a"));
        REQUIRE(IsAssociation<Public>(src, "R", "B", "b"));
        REQUIRE(IsAggregation<Public>(src, "R", "C", "c"));
        REQUIRE(IsAssociation<Public>(src, "R", "D", "d"));
        REQUIRE(IsAggregation<Public>(src, "R", "custom_container<E>", "e"));
        REQUIRE(IsAggregation<Public>(src, "R", "F", "f"));
        REQUIRE(IsAssociation<Public>(src, "R", "G", "g"));
        REQUIRE(IsAggregation<Public>(src, "R", "H", "h"));
        REQUIRE(IsAssociation<Public>(src, "R", "I", "i"));
        REQUIRE(IsAggregation<Public>(src, "R", "J", "j", "", "10"));
        REQUIRE(IsAssociation<Public>(src, "R", "K", "k", "", "20"));
        REQUIRE(IsAggregation<Public>(src, "R", "L", "lm"));
        REQUIRE(IsAggregation<Public>(src, "R", "M", "lm"));
        REQUIRE(IsAggregation<Public>(src, "R", "N", "ns"));
        REQUIRE(IsAggregation<Public>(src, "R", "NN", "ns"));
        REQUIRE(IsAggregation<Public>(src, "R", "NNN", "ns"));
    });
}