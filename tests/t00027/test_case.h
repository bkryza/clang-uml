/**
 * tests/t00027/test_case.cc
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

TEST_CASE("t00027")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00027", "t00027_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsAbstractClass(src, "Shape"));
        REQUIRE(IsAbstractClass(src, "ShapeDecorator"));

        REQUIRE(IsClassTemplate(src, "Line<T<>...>"));
        REQUIRE(IsInstantiation(src, "Line<T<>...>", "Line<Color>"));
        REQUIRE(IsInstantiation(src, "Line<T<>...>", "Line<Color,Weight>"));
        REQUIRE(
            IsAggregation<Public>(src, "Window", "Text<Color>", "description"));

        REQUIRE(IsInstantiation(src, "Line<T<>...>", "Line<Color>"));
        REQUIRE(IsInstantiation(src, "Line<T<>...>", "Line<Color,Weight>"));
        REQUIRE(IsInstantiation(src, "Text<T<>...>", "Text<Color>"));
        REQUIRE(IsInstantiation(src, "Text<T<>...>", "Text<Color,Weight>"));

        REQUIRE(IsAggregation<Public>(
            src, "Window", "Line<Color,Weight>", "border"));
        REQUIRE(IsAggregation<Public>(src, "Window", "Line<Color>", "divider"));
        REQUIRE(IsAggregation<Public>(
            src, "Window", "Text<Color,Weight>", "title"));
        REQUIRE(
            IsAggregation<Public>(src, "Window", "Text<Color>", "description"));
    });
}