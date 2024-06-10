/**
 * tests/t00036/test_case.cc
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

TEST_CASE("t00036")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00036", "t00036_class");

    REQUIRE(diagram->generate_packages() == true);

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClassTemplate(src, {"ns1::ns11", "A<T>"}));
        REQUIRE(IsClassTemplate(src, {"ns1::ns11", "A<int>"}));
        REQUIRE(IsClass(src, {"ns1::ns11::ns111", "B"}));
        REQUIRE(IsClass(src, {"ns2::ns22", "C"}));
        REQUIRE(IsEnum(src, {"ns1", "E"}));

        REQUIRE(!IsClass(src, "DImpl"));

        REQUIRE(IsNamespacePackage(src, "ns1"s));
        REQUIRE(IsNamespacePackage(src, "ns1"s, "ns11"s));
        REQUIRE(IsNamespacePackage(src, "ns1"s, "ns11"s, "ns111"s));
        REQUIRE(IsNamespacePackage(src, "ns2"s));
        REQUIRE(IsNamespacePackage(src, "ns2"s, "ns22"s));

        REQUIRE(!IsNamespacePackage(src, "ns3"s));
        REQUIRE(!IsNamespacePackage(src, "ns33"s));
        REQUIRE(!IsNamespacePackage(src, "ns3"s, "ns33"s));
    });
}