/**
 * tests/t00050/test_case.h
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

TEST_CASE("t00050")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00050", "t00050_class");

    CHECK_CLASS_DIAGRAM(
        *config, diagram, *model,
        [](const auto &src) {
            REQUIRE(IsClass(src, "A"));
            REQUIRE(IsClass(src, "B"));
            REQUIRE(IsClass(src, "C"));
            REQUIRE(IsClass(src, {"utils", "D"}));
            REQUIRE(IsEnum(src, "E"));

            REQUIRE(HasNote(src, "A", "left"));
            REQUIRE(HasNote(src, "A", "right"));
            REQUIRE(HasNote(src, "B", "top"));
            REQUIRE(HasNote(src, "C", "top"));
            REQUIRE(HasNote(src, "utils::D", "top"));
            REQUIRE(HasNote(src, "F<T,V,int N>", "top"));
            REQUIRE(HasNote(src, "G", "top"));
            REQUIRE(HasNote(src, "G", "bottom"));
            REQUIRE(HasNote(src, "G", "right"));
        },
        [](const plantuml_t &src) {
            REQUIRE(!HasNote(src, "E", "bottom"));
            REQUIRE(!HasNote(src, "NoComment", "top"));
        },
        [](const mermaid_t &src) {
            REQUIRE(!HasNote(src, "E", "bottom"));
            REQUIRE(!HasNote(src, "NoComment", "top"));
        });
}