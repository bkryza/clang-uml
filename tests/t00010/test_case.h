/**
 * tests/t00010/test_case.h
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

TEST_CASE("t00010")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00010", "t00010_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClassTemplate(src, "A<T,P>"));
        REQUIRE(IsClassTemplate(src, "B<T>"));

        REQUIRE(IsField<Public>(src, "B<T>", "astring", "A<T,std::string>"));
        REQUIRE(IsField<Public>(src, "C", "aintstring", "B<int>"));

        REQUIRE(IsInstantiation(src, "A<T,P>", "A<T,std::string>"));
        REQUIRE(IsInstantiation(src, "B<T>", "B<int>"));

        REQUIRE(
            IsAggregation<Public>(src, "B<T>", "A<T,std::string>", "astring"));
        REQUIRE(IsAggregation<Public>(src, "C", "B<int>", "aintstring"));
    });
}