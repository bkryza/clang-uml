/**
 * tests/t00086/test_case.h
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t00086")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00086", "t00086_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsObjCInterface(src, "t00086_a"));
        REQUIRE(IsInnerClass(src, "t00086_a", "t00086_a::Nested"));
        REQUIRE(IsEnum(src, "t00086_a::Color"));
        REQUIRE(IsClass(src, "t00086_a::(_flagSet)"));
        REQUIRE(IsUnion(src, "t00086_a::(_data)"));
        REQUIRE(IsClass(src, "t00086_a::(_data)::(_foo)"));
        REQUIRE(IsClass(src, "t00086_a::(_data)::(_bar)"));

        REQUIRE(IsAssociation<Public>(
            src, "t00086_a", "t00086_a::Nested", "_nested"));

        REQUIRE(IsAggregation<Public>(
            src, "t00086_a", "t00086_a::(_flagSet)", "_flagSet"));
        REQUIRE(IsAggregation<Public>(
            src, "t00086_a", "t00086_a::(_data)", "_data"));

        REQUIRE(IsAggregation<Public>(
            src, "t00086_a::(_data)", "t00086_a::(_data)::(_foo)", "_foo"));
        REQUIRE(IsAggregation<Public>(
            src, "t00086_a::(_data)", "t00086_a::(_data)::(_bar)", "_bar"));
    });
}