/**
 * tests/t00032/test_case.h
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

TEST_CASE("t00032")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00032", "t00032_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "Base"));
        REQUIRE(IsClass(src, "TBase"));
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, "C"));
        REQUIRE(IsClass(src, "R"));

        REQUIRE(IsClassTemplate(src, "Overload<T,L,Ts...>"));

        REQUIRE(IsBaseClass(src, "Base", "Overload<T,L,Ts...>"));
        REQUIRE(IsBaseClass(src, "TBase", "Overload<TBase,int,A,B,C>"));
        REQUIRE(IsBaseClass(src, "A", "Overload<TBase,int,A,B,C>"));
        REQUIRE(IsBaseClass(src, "B", "Overload<TBase,int,A,B,C>"));
        REQUIRE(IsBaseClass(src, "C", "Overload<TBase,int,A,B,C>"));
        REQUIRE(!IsDependency(src, "Overload<TBase,int,A,B,C>", "TBase"));
        REQUIRE(!IsDependency(src, "Overload<TBase,int,A,B,C>", "A"));
        REQUIRE(!IsDependency(src, "Overload<TBase,int,A,B,C>", "B"));
        REQUIRE(!IsDependency(src, "Overload<TBase,int,A,B,C>", "C"));
    });
}