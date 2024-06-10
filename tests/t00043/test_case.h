/**
 * tests/t00043/test_case.h
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

TEST_CASE("t00043")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00043", "t00043_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, {"dependants", "A"}));
        REQUIRE(IsClass(src, {"dependants", "B"}));
        REQUIRE(IsClass(src, {"dependants", "C"}));
        REQUIRE(IsClass(src, {"dependants", "D"}));
        REQUIRE(IsClass(src, {"dependants", "BB"}));
        REQUIRE(IsClass(src, {"dependants", "E"}));
        REQUIRE(!IsClass(src, {"dependants", "EE"}));
        REQUIRE(!IsClass(src, {"dependants", "EEE"}));

        REQUIRE(IsDependency(src, {"dependants", "B"}, {"dependants", "A"}));
        REQUIRE(IsDependency(src, {"dependants", "BB"}, {"dependants", "A"}));
        REQUIRE(IsDependency(src, {"dependants", "C"}, {"dependants", "B"}));
        REQUIRE(IsDependency(src, {"dependants", "D"}, {"dependants", "C"}));
        REQUIRE(IsDependency(src, {"dependants", "E"}, {"dependants", "D"}));

        REQUIRE(IsClass(src, {"dependencies", "G"}));
        REQUIRE(IsClass(src, {"dependencies", "GG"}));
        REQUIRE(IsClass(src, {"dependencies", "H"}));
        REQUIRE(!IsClass(src, {"dependencies", "HH"}));
        REQUIRE(!IsClass(src, {"dependencies", "II"}));
        REQUIRE(!IsClass(src, {"dependencies", "III"}));

        REQUIRE(
            IsDependency(src, {"dependencies", "J"}, {"dependencies", "I"}));
        REQUIRE(
            IsDependency(src, {"dependencies", "H"}, {"dependencies", "G"}));
        REQUIRE(
            IsDependency(src, {"dependencies", "I"}, {"dependencies", "H"}));
        REQUIRE(
            IsDependency(src, {"dependencies", "H"}, {"dependencies", "GG"}));
    });
}