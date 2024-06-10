/**
 * tests/t30003/test_case.h
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

TEST_CASE("t30003")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_PACKAGE_MODEL("t30003", "t30003_package");

    CHECK_PACKAGE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsNamespacePackage(src, "ns1"s));
        REQUIRE(IsNamespacePackage(src, "ns1"s, "ns2_v1_0_0"s));
        REQUIRE(IsNamespacePackage(src, "ns1"s, "ns2_v0_9_0"s));
        REQUIRE(IsNamespacePackage(src, "ns3"s));
        REQUIRE(IsNamespacePackage(src, "ns3"s, "ns1"s));
        REQUIRE(IsNamespacePackage(src, "ns3"s, "ns1"s, "ns2"s));

        REQUIRE(IsDeprecated(src, "ns2_v0_9_0"));
        REQUIRE(IsDeprecated(src, "ns3"));
    });
}
