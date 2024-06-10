/**
 * tests/t00020/test_case.h
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

TEST_CASE("t00020")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00020", "t00020_class");

    CHECK_CLASS_DIAGRAM(
        *config, diagram, *model,
        [](const auto &src) {
            REQUIRE(IsAbstractClass(src, "AbstractFactory"));
            REQUIRE(IsAbstractClass(src, "ProductA"));
            REQUIRE(IsAbstractClass(src, "ProductB"));
            REQUIRE(IsClass(src, "ProductA1"));
            REQUIRE(IsClass(src, "ProductA2"));
            REQUIRE(IsClass(src, "ProductB1"));
            REQUIRE(IsClass(src, "ProductB2"));
            REQUIRE(IsClass(src, "Factory1"));
            REQUIRE(IsClass(src, "Factory2"));
        },
        [](const plantuml_t &src) {
            REQUIRE(IsDependency(src, "Factory1", "ProductA1"));
            REQUIRE(IsDependency(src, "Factory1", "ProductB1"));
            REQUIRE(IsDependency(src, "Factory2", "ProductA2"));
            REQUIRE(IsDependency(src, "Factory2", "ProductB2"));
        },
        [](const mermaid_t &src) {
            REQUIRE(IsDependency(src, "Factory1", "ProductA1"));
            REQUIRE(IsDependency(src, "Factory1", "ProductB1"));
            REQUIRE(IsDependency(src, "Factory2", "ProductA2"));
            REQUIRE(IsDependency(src, "Factory2", "ProductB2"));
        });
}