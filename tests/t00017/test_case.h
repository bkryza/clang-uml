/**
 * tests/t00017/test_case.h
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

TEST_CASE("t00017")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00017", "t00017_class");

    CHECK_CLASS_DIAGRAM(
        *config, diagram, *model,
        [](const auto &src) {
            REQUIRE(IsClass(src, "A"));
            REQUIRE(IsClass(src, "B"));
            REQUIRE(IsClass(src, "C"));
            REQUIRE(IsClass(src, "D"));
            REQUIRE(IsClass(src, "E"));
            REQUIRE(IsClass(src, "F"));
            REQUIRE(IsClass(src, "G"));
            REQUIRE(IsClass(src, "H"));
            REQUIRE(IsClass(src, "I"));
            REQUIRE(IsClass(src, "J"));
            REQUIRE(IsClass(src, "K"));
            REQUIRE(IsClass(src, "R"));

            REQUIRE(IsField<Private>(src, "R", "some_int", "int"));
            REQUIRE((IsField<Private>(src, "R", "some_int_pointer", "int *")));
            REQUIRE((IsField<Private>(
                src, "R", "some_int_pointer_pointer", "int **")));

            REQUIRE(IsAggregation<Private>(src, "R", "A", "a"));
            REQUIRE(IsAssociation<Private>(src, "R", "B", "b"));
            REQUIRE(IsAssociation<Private>(src, "R", "C", "c"));
            REQUIRE(IsAssociation<Private>(src, "R", "D", "d"));
            REQUIRE(IsAssociation<Private>(src, "R", "E", "e"));
            REQUIRE(IsAggregation<Private>(src, "R", "F", "f"));
            REQUIRE(IsAssociation<Private>(src, "R", "G", "g"));
            REQUIRE(IsAssociation<Private>(src, "R", "H", "h"));
            REQUIRE(IsAssociation<Private>(src, "R", "I", "i"));
            REQUIRE(IsAssociation<Private>(src, "R", "J", "j"));
            REQUIRE(IsAssociation<Private>(src, "R", "K", "k"));
        },
        [](const plantuml_t &src) {
            // Relationship members should not be rendered as part of this
            // testcase
            REQUIRE(!IsField<Private>(src, "R", "a", "A"));
            REQUIRE(!IsField<Private>(src, "R", "b", "B"));
        },
        [](const mermaid_t &src) {
            REQUIRE(!IsField<Private>(src, "R", "a", "A"));
            REQUIRE(!IsField<Private>(src, "R", "b", "B"));
        });
}