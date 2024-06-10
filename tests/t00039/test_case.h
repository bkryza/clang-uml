/**
 * tests/t00039/test_case.h
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

TEST_CASE("t00039")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00039", "t00039_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "AA"));
        REQUIRE(IsClass(src, "AAA"));
        REQUIRE(IsClass(src, "ns2::AAAA"));
        REQUIRE(IsBaseClass(src, "A", "AA"));
        REQUIRE(IsBaseClass(src, "AA", "AAA"));
        REQUIRE(IsBaseClass(src, "AAA", "ns2::AAAA"));
        REQUIRE(!IsClass(src, "detail::AA"));

        REQUIRE(!IsClass(src, "B"));
        REQUIRE(!IsClass(src, "ns1::BB"));

        REQUIRE(IsClass(src, "C"));
        REQUIRE(IsClass(src, "D"));
        REQUIRE(IsClass(src, "E"));
        REQUIRE(IsBaseClass(src, "C", "CD"));
        REQUIRE(IsBaseClass(src, "D", "CD"));
        REQUIRE(IsBaseClass(src, "D", "DE"));
        REQUIRE(IsBaseClass(src, "E", "DE"));
        REQUIRE(IsBaseClass(src, "C", "CDE"));
        REQUIRE(IsBaseClass(src, "D", "CDE"));
        REQUIRE(IsBaseClass(src, "E", "CDE"));

        REQUIRE(IsClassTemplate(src, "ns3::F<T>"));
        REQUIRE(IsClassTemplate(src, "ns3::FF<T,M>"));
        REQUIRE(IsClassTemplate(src, "ns3::FE<T,M>"));
        REQUIRE(IsClassTemplate(src, "ns3::FFF<T,M,N>"));
    });
}
