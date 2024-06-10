/**
 * tests/t00009/test_case.h
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

TEST_CASE("t00009")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00009", "t00009_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClassTemplate(src, "A<T>"));
        REQUIRE(IsClass(src, "B"));

        REQUIRE(IsField<Public>(src, "A<T>", "value", "T"));
        REQUIRE(IsField<Public>(src, "B", "aint", "A<int>"));
        REQUIRE(IsField<Public>(src, "B", "astring", "A<std::string> *"));
        REQUIRE(IsField<Public>(
            src, "B", "avector", "A<std::vector<std::string>> &"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<int>", "up"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<std::string>", "up"));

        REQUIRE(
            IsAggregation<Public>(src, "B", "A<int>", "aint", "", "", "up"));
        REQUIRE(IsAssociation<Public>(
            src, "B", "A<std::string>", "astring", "", "", "up"));
        REQUIRE(IsAssociation<Public>(
            src, "B", "A<std::vector<std::string>>", "avector", "", "", "up"));
    });
}