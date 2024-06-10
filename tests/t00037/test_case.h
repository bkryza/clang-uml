/**
 * tests/t00037/test_case.h
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

TEST_CASE("t00037")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00037", "t00037_class");

    REQUIRE(diagram->generate_packages() == true);

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "ST"));
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "ST::(units)"));
        REQUIRE(IsClass(src, "ST::(dimensions)"));
        REQUIRE(IsClass(src, "ST::(bars)"));

        REQUIRE(
            IsAggregation<Public>(src, "ST", "ST::(dimensions)", "dimensions"));
        REQUIRE(IsAggregation<Private>(src, "ST", "ST::(units)", "units"));
        REQUIRE(
            IsAggregation<Public>(src, "ST", "ST::(bars)", "bars", "", "10"));
        REQUIRE(IsAggregation<Private>(
            src, "ST", "S", "s", "", std::to_string(4 * 3 * 2)));

        REQUIRE(IsField<Private>(src, "ST", "s", "S[4][3][2]"));

        REQUIRE(IsField<Public>(src, "ST", "bars", "ST::(bars)[10]"));
        REQUIRE(IsField<Public>(src, "ST", "dimensions", "ST::(dimensions)"));
        REQUIRE(IsField<Private>(src, "ST", "units", "ST::(units)"));
    });
}