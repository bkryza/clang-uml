/**
 * tests/t00090/test_case.h
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

TEST_CASE("t00090")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00090", "t00090_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(!src.contains("type-parameter-"));

        REQUIRE(IsClassTemplate(src, "HList<>"));
        REQUIRE(IsClassTemplate(src, "HList<T...>"));
        REQUIRE(IsClassTemplate(src, "HList<T0,TRest...>"));
        REQUIRE(IsClassTemplate(src, "HList<TRest...>"));

        REQUIRE(IsClassTemplate(src, "Arithmetic<IsArithmetic T...>"));

        REQUIRE(IsConcept(src, "IsArithmetic<T>"));

        REQUIRE(IsBaseClass(src, "HList<TRest...>", "HList<T0,TRest...>"));
        REQUIRE(
            IsBaseClass(src, "HList<T...>", "Arithmetic<IsArithmetic T...>"));

        REQUIRE(IsInstantiation(src, "HList<T...>", "HList<T0,TRest...>"));
        REQUIRE(IsInstantiation(src, "HList<T...>", "HList<TRest...>"));
        REQUIRE(IsInstantiation(src, "HList<TRest...>", "HList<T...>"));

        if constexpr (std::is_same_v<plantuml_t, std::decay_t<decltype(src)>>) {
            REQUIRE(IsInstantiation(src, "HList<T...>", "HList< >"));
        }
        else {
            REQUIRE(IsInstantiation(src, "HList<T...>", "HList<>"));
        }
    });
}