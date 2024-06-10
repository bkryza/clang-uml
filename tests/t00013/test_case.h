/**
 * tests/t00013/test_case.h
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

TEST_CASE("t00013")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00013", "t00013_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, "C"));
        REQUIRE(IsClass(src, "D"));
        REQUIRE(IsClassTemplate(src, "E<T>"));
        REQUIRE(IsClassTemplate(src, "G<T,Args...>"));

        REQUIRE(!IsDependency(src, "R", "R"));
        REQUIRE(IsDependency(src, "R", "A"));
        REQUIRE(IsDependency(src, "R", "B"));
        REQUIRE(IsDependency(src, "R", "C"));
        REQUIRE(IsDependency(src, "R", "D"));
        REQUIRE(IsDependency(src, "D", "R"));
        REQUIRE(IsDependency(src, "R", "E<T>"));
        REQUIRE(IsDependency(src, "R", "E<int>"));
        REQUIRE(IsInstantiation(src, "E<T>", "E<int>"));
        REQUIRE(IsInstantiation(src, "E<T>", "E<std::string>"));
        REQUIRE(IsAggregation<Private>(src, "R", "E<std::string>", "estring"));
        REQUIRE(IsDependency(src, "R", "ABCD::F<T>"));
        REQUIRE(IsInstantiation(src, "ABCD::F<T>", "ABCD::F<int>"));
        REQUIRE(IsDependency(src, "R", "ABCD::F<int>"));
        REQUIRE(
            IsInstantiation(src, "G<T,Args...>", "G<int,float,std::string>"));
    });
}