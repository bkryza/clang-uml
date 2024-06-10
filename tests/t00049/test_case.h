/**
 * tests/t00049/test_case.h
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

TEST_CASE("t00049")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00049", "t00049_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "R"));

        REQUIRE(IsClassTemplate(src, "A<T>"));

        REQUIRE(IsMethod<Public>(src, "R", "get_int_map", "A<intmap>"));
        REQUIRE(IsMethod<Public>(
            src, "R", "set_int_map", "void", "A<intmap> && int_map"));

        REQUIRE(IsField<Public>(src, "R", "a_string", "A<thestring>"));
        REQUIRE(
            IsField<Public>(src, "R", "a_vector_string", "A<string_vector>"));
        REQUIRE(IsField<Public>(src, "R", "a_int_map", "A<intmap>"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<string_vector>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<thestring>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<intmap>"));
    });
}