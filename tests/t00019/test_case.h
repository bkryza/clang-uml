/**
 * tests/t00019/test_case.h
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

TEST_CASE("t00019")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00019", "t00019_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "Base"));
        REQUIRE(IsClassTemplate(src, "Layer1<LowerLayer>"));
        REQUIRE(IsClassTemplate(src, "Layer2<LowerLayer>"));
        REQUIRE(IsClassTemplate(src, "Layer3<LowerLayer>"));

        REQUIRE(IsBaseClass(src, "Base", "Layer3<Base>"));
        REQUIRE(!IsDependency(src, "Base", "Layer3<Base>"));

        REQUIRE(IsBaseClass(src, "Layer3<Base>", "Layer2<Layer3<Base>>"));
        REQUIRE(!IsDependency(src, "Layer3<Base>", "Layer2<Layer3<Base>>"));

        REQUIRE(IsBaseClass(
            src, "Layer2<Layer3<Base>>", "Layer1<Layer2<Layer3<Base>>>"));

        REQUIRE(!IsDependency(
            src, "Layer2<Layer3<Base>>", "Layer1<Layer2<Layer3<Base>>>"));

        REQUIRE(IsAggregation<Public>(
            src, "A", "Layer1<Layer2<Layer3<Base>>>", "layers"));
        REQUIRE(!IsDependency(src, "A", "Layer1<Layer2<Layer3<Base>>>"));

        REQUIRE(
            !IsAggregation<Public>(src, "A", "Layer2<Layer3<Base>>", "layers"));

        REQUIRE(!IsAggregation<Public>(src, "A", "Layer3<Base>", "layers"));

        REQUIRE(!IsAggregation<Public>(src, "A", "Base", "layers"));
    });
}