/**
 * tests/t00038/test_case.h
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

TEST_CASE("t00038")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00038", "t00038_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, "C"));
        REQUIRE(IsClass(src, "thirdparty::ns1::E"));
        REQUIRE(IsClass(src, "key_t"));
        REQUIRE(IsClassTemplate(src, "map<T>"));
        REQUIRE(IsClassTemplate(src,
            "map<std::integral_constant<property_t,property_t::property_a>>"));

        REQUIRE(IsClassTemplate(src,
            "map<std::vector<std::integral_constant<property_t,property_t::"
            "property_b>>>"));
        REQUIRE(IsClassTemplate(src,
            "map<std::map<key_t,std::vector<std::integral_constant<property_t,"
            "property_t::property_c>>>>"));

        REQUIRE(IsEnum(src, "property_t"));

        REQUIRE(IsInstantiation(src, "map<T>",
            "map<std::map<key_t,std::vector<std::integral_constant<"
            "property_t,property_t::property_c>>>>"));

        REQUIRE(IsDependency(src,
            "map<std::integral_constant<property_t,property_t::property_a>>",
            "property_t"));

        REQUIRE(IsDependency(src,
            "map<"
            "std::vector<std::integral_constant<property_t,"
            "property_t::property_b>>>",
            "property_t"));

        REQUIRE(IsDependency(src,
            "map<std::map<key_t,std::vector<std::integral_constant<"
            "property_t,property_t::property_c>>>>",
            "property_t"));

        REQUIRE(IsDependency(src,
            "map<std::map<key_t,std::vector<std::integral_constant<"
            "property_t,property_t::property_c>>>>",
            "key_t"));

        REQUIRE(IsDependency(src,
            "map<std::integral_constant<thirdparty::ns1::color_t,"
            "thirdparty::ns1::color_t::red>>",
            "thirdparty::ns1::color_t"));

        REQUIRE(IsBaseClass(src, "thirdparty::ns1::E",
            "map<std::integral_constant<thirdparty::ns1::color_t,"
            "thirdparty::ns1::color_t::red>>"));
    });
}