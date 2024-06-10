/**
 * tests/t00029/test_case.cc
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

TEST_CASE("t00029")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00029", "t00029_class");

    REQUIRE(model->name() == "t00029_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(!IsClass(src, "B"));
        REQUIRE(IsClassTemplate(src, "C<T>"));
        REQUIRE(!IsClassTemplate(src, "D<T>"));
        REQUIRE(IsEnum(src, "E"));
        REQUIRE(!IsEnum(src, "F"));
        REQUIRE(IsClass(src, "G1"));
        REQUIRE(IsClass(src, "G2"));
        REQUIRE(IsClass(src, "G3"));
        REQUIRE(IsClass(src, "G4"));

        REQUIRE(IsClass(src, "R"));

        REQUIRE(IsAggregation<Public>(src, "R", "G1", "g1"));
        REQUIRE(!IsAggregation<Public>(src, "R", "G2", "g2"));
        REQUIRE(!IsAggregation<Public>(src, "R", "G3", "g3"));
        REQUIRE(IsAssociation<Public>(src, "R", "G4", "g4"));
    });
}