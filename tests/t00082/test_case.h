/**
 * tests/t00082/test_case.h
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

TEST_CASE("t00082")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00082", "t00082_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, {"ns1::nsA", "A1"}));
        REQUIRE(IsClass(src, {"ns1::nsA", "B1"}));
        REQUIRE(IsClass(src, {"ns1::nsA", "C1"}));
        REQUIRE(!IsClass(src, {"ns1::nsA", "D1"}));

        REQUIRE(IsClass(src, {"ns2::nsB", "A2"}));
        REQUIRE(IsClass(src, {"ns2::nsB", "B2"}));
        REQUIRE(IsClass(src, {"ns2::nsB", "C2"}));

        REQUIRE(IsClass(src, {"ns3::nsC", "A3"}));
        REQUIRE(IsClass(src, {"ns3::nsC", "B3"}));
        REQUIRE(IsClass(src, {"ns3::nsC", "C3"}));
        REQUIRE(!IsClass(src, {"ns3::nsC", "D3"}));

        REQUIRE(!IsNamespacePackage(src, "ns4"s));

        REQUIRE(!IsClass(src, {"ns4::nsD", "A4"}));
    });
}