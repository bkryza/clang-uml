/**
 * tests/t00089/test_case.h
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

TEST_CASE("t00089")
{
    using namespace clanguml::test;
    using namespace std::string_literals;
    {
        auto [config, db, diagram, model] =
            CHECK_CLASS_MODEL("t00089", "t00089_class");

        CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
            REQUIRE(IsClass(src, "::t00089_A"));
            REQUIRE(IsEnum(src, "::t00089_E"));

            REQUIRE(IsClass(src, "::thirdparty::lib1::A"));
            REQUIRE(IsClass(src, "::thirdparty::lib2::B"));
            REQUIRE(IsClass(src, "thirdparty::lib1::A"));

            REQUIRE(IsClass(src, "thirdparty::lib3::C"));
            REQUIRE(IsClass(src, "thirdparty::lib4::D"));
        });
    }
    {
        auto [config, db, diagram, model] =
            CHECK_CLASS_MODEL("t00089", "t00089_packages_class");

        CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
            REQUIRE(IsClass(src, "::t00089_A"));
            REQUIRE(IsEnum(src, "::t00089_E"));
            REQUIRE(IsClass(src, {"::thirdparty::lib1", "A"}));
            REQUIRE(IsClass(src, {"::thirdparty::lib2", "B"}));
            REQUIRE(IsClass(src, {"thirdparty::lib1", "A"}));

            REQUIRE(IsClass(src, {"thirdparty::lib3", "C"}));
            REQUIRE(IsClass(src, {"thirdparty::lib4", "D"}));
        });
    }
}