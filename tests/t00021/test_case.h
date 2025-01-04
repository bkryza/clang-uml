/**
 * tests/t00021/test_case.h
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

TEST_CASE("t00021")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00021", "t00021_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsAbstractClass(src, "Item"));
        REQUIRE(IsAbstractClass(src, "Visitor"));
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));

        REQUIRE(IsClass(src, "Visitor1"));
        REQUIRE(IsClass(src, "Visitor2"));
        REQUIRE(IsClass(src, "Visitor3"));
    });
}