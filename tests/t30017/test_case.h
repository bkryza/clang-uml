/**
 * tests/t30017/test_case.h
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

TEST_CASE("t30017")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_PACKAGE_MODEL("t30017", "t30017_package");

    CHECK_PACKAGE_DIAGRAM(
        *config, diagram, *model,
        [](const auto &src) {
            REQUIRE(IsNamespacePackage(src, "::thirdparty"s, "lib1"s));
            REQUIRE(IsNamespacePackage(src, "::thirdparty"s, "lib2"s));
            REQUIRE(IsNamespacePackage(src, "thirdparty"s, "lib1"s));
            REQUIRE(IsNamespacePackage(src, "thirdparty"s, "lib3"s));
            REQUIRE(IsNamespacePackage(src, "thirdparty"s, "lib4"s));
        },
        [](const json_t &src) {
            REQUIRE(IsDependency(
                src, {"thirdparty", "lib3"}, {"::thirdparty", "lib2"}));
        });
}