/**
 * tests/t00092/test_case.h
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

TEST_CASE("t00092")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00092", "t00092_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, "C"));
        REQUIRE(IsClass(src, "R"));
        REQUIRE(IsClass(src, "String"));

        REQUIRE(IsDependency(src, "R", "String"));
        REQUIRE(IsAggregation<Public>(src, "R", "A", "a"));
        REQUIRE(IsAssociation<Public>(src, "R", "B", "b"));
        REQUIRE(IsAggregation<Public>(src, "R", "C", "m"));
    });
}