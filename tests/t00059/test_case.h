/**
 * tests/t00059/test_case.h
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

TEST_CASE("t00059")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00059", "t00059_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsConcept(src, "fruit_c<T>"));
        REQUIRE(IsConcept(src, "apple_c<T>"));
        REQUIRE(IsConcept(src, "orange_c<T>"));

        REQUIRE(IsConstraint(src, "apple_c<T>", "fruit_c<T>", "T", "up"));
        REQUIRE(IsConstraint(src, "orange_c<T>", "fruit_c<T>", "T", "up"));

        REQUIRE(IsConceptRequirement(src, "apple_c<T>", "t.get_sweetness()"));
        REQUIRE(IsConceptRequirement(src, "orange_c<T>", "t.get_bitterness()"));

        REQUIRE(IsClass(src, "gala_apple"));
        REQUIRE(IsClass(src, "empire_apple"));
        REQUIRE(IsClass(src, "valencia_orange"));
        REQUIRE(IsClass(src, "lima_orange"));
        REQUIRE(IsClass(src, "R"));

        REQUIRE(IsClassTemplate(src, "fruit_factory<apple_c TA,orange_c TO>"));

        REQUIRE(IsDependency(src, "fruit_factory<gala_apple,valencia_orange>",
            "gala_apple", "up"));
        REQUIRE(IsDependency(src, "fruit_factory<gala_apple,valencia_orange>",
            "valencia_orange", "up"));

        REQUIRE(IsDependency(src, "fruit_factory<empire_apple,lima_orange>",
            "empire_apple", "up"));
        REQUIRE(IsDependency(src, "fruit_factory<empire_apple,lima_orange>",
            "lima_orange", "up"));

        REQUIRE(IsAggregation<Public>(src, "R",
            "fruit_factory<gala_apple,valencia_orange>", "factory_1", "", "",
            "up"));
        REQUIRE(IsAggregation<Public>(src, "R",
            "fruit_factory<empire_apple,lima_orange>", "factory_2", "", "",
            "up"));

        REQUIRE(IsInstantiation(src, "fruit_factory<apple_c TA,orange_c TO>",
            "fruit_factory<gala_apple,valencia_orange>", "up"));
        REQUIRE(IsInstantiation(src, "fruit_factory<apple_c TA,orange_c TO>",
            "fruit_factory<empire_apple,lima_orange>", "up"));
    });
}