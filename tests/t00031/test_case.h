/**
 * tests/t00031/test_case.h
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

TEST_CASE("t00031")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00031", "t00031_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsEnum(src, "B"));
        REQUIRE(IsClass(src, "D"));
        REQUIRE(IsClassTemplate(src, "C<T>"));

        REQUIRE(IsAssociation<Public>(
            src, "R", "A", "aaa", "", "", "[#red,dashed,thickness=2]"));
        REQUIRE(IsComposition<Public>(
            src, "R", "B", "bbb", "", "", "[#green,dashed,thickness=4]"));
        REQUIRE(IsDependency(src, "R", "B"));
        REQUIRE(IsAggregation<Public>(
            src, "R", "C<int>", "ccc", "", "", "[#blue,dotted,thickness=8]"));
        REQUIRE(IsAssociation<Public>(
            src, "R", "D", "ddd", "", "", "[#blue,plain,thickness=16]"));
    });
}