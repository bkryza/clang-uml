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

TEST_CASE("t30013", "[test-case][package]")
{
    auto [config, db] = load_config("t30013");

    auto diagram = config.diagrams["t30013_package"];

    REQUIRE(diagram->name == "t30013_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30013_package");

    {
        auto src = generate_package_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all packages exist
        REQUIRE_THAT(src, IsPackage("app"));
        REQUIRE_THAT(src, IsPackage("mod1"));
        REQUIRE_THAT(src, IsPackage("mod2"));
        REQUIRE_THAT(src, IsPackage("mod3"));
        REQUIRE_THAT(src, IsPackage("mod4"));
        REQUIRE_THAT(src, IsPackage("mod5"));
        REQUIRE_THAT(src, IsPackage("mod6"));
        REQUIRE_THAT(src, IsPackage("mod7"));
        REQUIRE_THAT(src, IsPackage("mod8"));
        REQUIRE_THAT(src, IsPackage("mod9"));
        REQUIRE_THAT(src, IsPackage("mod10"));
        REQUIRE_THAT(src, IsPackage("mod11"));
        REQUIRE_THAT(src, IsPackage("mod12"));
        REQUIRE_THAT(src, IsPackage("mod13"));
        REQUIRE_THAT(src, IsPackage("mod14"));
        REQUIRE_THAT(src, IsPackage("mod15"));
        REQUIRE_THAT(src, IsPackage("mod16"));
        REQUIRE_THAT(src, IsPackage("mod17"));
        REQUIRE_THAT(src, IsPackage("mod18"));

        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod1")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod2")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod3")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod4")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod5")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod6")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod7")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod8")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod9")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod10")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod11")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod12")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod13")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod14")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod15")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod16")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod17")));
        REQUIRE_THAT(src, IsDependency(_A("app"), _A("mod18")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;
        using namespace std::string_literals;
        REQUIRE(IsModulePackage(j, "app"s));
        REQUIRE(IsModulePackage(j, "mod1"s));
        REQUIRE(IsModulePackage(j, "mod2"s));
        REQUIRE(IsModulePackage(j, "mod3"s));
        REQUIRE(IsModulePackage(j, "mod4"s));
        REQUIRE(IsModulePackage(j, "mod5"s));
        REQUIRE(IsModulePackage(j, "mod6"s));
        REQUIRE(IsModulePackage(j, "mod7"s));
        REQUIRE(IsModulePackage(j, "mod8"s));
        REQUIRE(IsModulePackage(j, "mod9"s));
        REQUIRE(IsModulePackage(j, "mod10"s));
        REQUIRE(IsModulePackage(j, "mod11"s));
        REQUIRE(IsModulePackage(j, "mod12"s));
        REQUIRE(IsModulePackage(j, "mod13"s));
        REQUIRE(IsModulePackage(j, "mod14"s));
        REQUIRE(IsModulePackage(j, "mod15"s));
        REQUIRE(IsModulePackage(j, "mod16"s));
        REQUIRE(IsModulePackage(j, "mod17"s));
        REQUIRE(IsModulePackage(j, "mod18"s));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_package_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsPackage;
        using mermaid::IsPackageDependency;

        REQUIRE_THAT(src, IsPackage(_A("app")));
        REQUIRE_THAT(src, IsPackage(_A("mod1")));
        REQUIRE_THAT(src, IsPackage(_A("mod2")));
        REQUIRE_THAT(src, IsPackage(_A("mod3")));
        REQUIRE_THAT(src, IsPackage(_A("mod4")));
        REQUIRE_THAT(src, IsPackage(_A("mod5")));
        REQUIRE_THAT(src, IsPackage(_A("mod6")));
        REQUIRE_THAT(src, IsPackage(_A("mod7")));
        REQUIRE_THAT(src, IsPackage(_A("mod8")));
        REQUIRE_THAT(src, IsPackage(_A("mod9")));
        REQUIRE_THAT(src, IsPackage(_A("mod10")));
        REQUIRE_THAT(src, IsPackage(_A("mod11")));
        REQUIRE_THAT(src, IsPackage(_A("mod12")));
        REQUIRE_THAT(src, IsPackage(_A("mod13")));
        REQUIRE_THAT(src, IsPackage(_A("mod14")));
        REQUIRE_THAT(src, IsPackage(_A("mod15")));
        REQUIRE_THAT(src, IsPackage(_A("mod16")));
        REQUIRE_THAT(src, IsPackage(_A("mod17")));
        REQUIRE_THAT(src, IsPackage(_A("mod18")));

        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod1")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod2")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod3")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod4")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod5")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod6")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod7")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod8")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod9")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod10")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod11")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod12")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod13")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod14")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod15")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod16")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod17")));
        REQUIRE_THAT(src, IsPackageDependency(_A("app"), _A("mod18")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}