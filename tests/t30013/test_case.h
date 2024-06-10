/**
 * tests/t30013/test_case.h
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

TEST_CASE("t30013")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_PACKAGE_MODEL("t30013", "t30013_package");

    CHECK_PACKAGE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsModulePackage(src, "app"s));
        REQUIRE(IsModulePackage(src, "mod1"s));
        REQUIRE(IsModulePackage(src, "mod2"s));
        REQUIRE(IsModulePackage(src, "mod3"s));
        REQUIRE(IsModulePackage(src, "mod4"s));
        REQUIRE(IsModulePackage(src, "mod5"s));
        REQUIRE(IsModulePackage(src, "mod6"s));
        REQUIRE(IsModulePackage(src, "mod7"s));
        REQUIRE(IsModulePackage(src, "mod8"s));
        REQUIRE(IsModulePackage(src, "mod9"s));
        REQUIRE(IsModulePackage(src, "mod10"s));
        REQUIRE(IsModulePackage(src, "mod11"s));
        REQUIRE(IsModulePackage(src, "mod12"s));
        REQUIRE(IsModulePackage(src, "mod13"s));
        REQUIRE(IsModulePackage(src, "mod14"s));
        REQUIRE(IsModulePackage(src, "mod15"s));
        REQUIRE(IsModulePackage(src, "mod16"s));
        REQUIRE(IsModulePackage(src, "mod17"s));
        REQUIRE(IsModulePackage(src, "mod18"s));

        REQUIRE(IsDependency(src, "app", "mod1"));
        REQUIRE(IsDependency(src, "app", "mod2"));
        REQUIRE(IsDependency(src, "app", "mod3"));
        REQUIRE(IsDependency(src, "app", "mod4"));
        REQUIRE(IsDependency(src, "app", "mod5"));
        REQUIRE(IsDependency(src, "app", "mod6"));
        REQUIRE(IsDependency(src, "app", "mod7"));
        REQUIRE(IsDependency(src, "app", "mod8"));
        REQUIRE(IsDependency(src, "app", "mod9"));
        REQUIRE(IsDependency(src, "app", "mod10"));
        REQUIRE(IsDependency(src, "app", "mod11"));
        REQUIRE(IsDependency(src, "app", "mod12"));
        REQUIRE(IsDependency(src, "app", "mod13"));
        REQUIRE(IsDependency(src, "app", "mod14"));
        REQUIRE(IsDependency(src, "app", "mod15"));
        REQUIRE(IsDependency(src, "app", "mod16"));
        REQUIRE(IsDependency(src, "app", "mod17"));
        REQUIRE(IsDependency(src, "app", "mod18"));
    });
}