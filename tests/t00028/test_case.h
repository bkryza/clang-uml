/**
 * tests/t00028/test_case.h
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

TEST_CASE("t00028")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00028", "t00028_class");

    CHECK_CLASS_DIAGRAM(
        *config, diagram, *model,
        [](const auto &src) {
            REQUIRE(IsClass(src, "A"));
            REQUIRE(IsClass(src, "B"));
            REQUIRE(IsClass(src, "C"));
            REQUIRE(IsClass(src, "D"));
            REQUIRE(IsClassTemplate(src, "E<T>"));
            REQUIRE(IsEnum(src, "F"));
            REQUIRE(IsClass(src, "R"));
            REQUIRE(HasNote(src, "A", "top", "A class note."));
            REQUIRE(HasNote(src, "B", "left", "B class note."));
            REQUIRE(HasNote(src, "C", "bottom", "C class note."));
            const auto d_note = R"(
D
class
note.)";
            REQUIRE(HasNote(src, "D", "left", d_note));
            REQUIRE(HasNote(src, "E<T>", "left", "E template class note."));
            REQUIRE(HasNote(src, "F", "bottom", "F enum note."));
            REQUIRE(HasNote(src, "R", "right", "R class note."));
        },
        [](const plantuml_t &src) {
            REQUIRE(HasMemberNote(src, "R", "ccc", "left", "Reference to C."));
            REQUIRE(!HasMemberNote(src, "R", "bbb", "right", "R class note."));
            REQUIRE(HasMemberNote(
                src, "R", "aaa", "left", "R contains an instance of A."));
            REQUIRE(!HasNote(src, "G", "left", "G class note."));
        },
        [](const mermaid_t &src) {
            REQUIRE(HasNote(src, "R", "left", "R contains an instance of A."));
            REQUIRE(!HasNote(src, "G", "left", "G class note."));
        });
}