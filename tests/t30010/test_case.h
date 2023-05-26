/**
 * tests/t30010/test_case.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t30010", "[test-case][package]")
{
    auto [config, db] = load_config("t30010");

    auto diagram = config.diagrams["t30010_package"];

    REQUIRE(diagram->name == "t30010_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30010_package");

    {
        auto puml = generate_package_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        REQUIRE_THAT(puml, IsPackage("app"));
        REQUIRE_THAT(puml, IsPackage("libraries"));
        REQUIRE_THAT(puml, IsPackage("lib1"));
        REQUIRE_THAT(puml, IsPackage("lib2"));
        REQUIRE_THAT(puml, !IsPackage("library1"));
        REQUIRE_THAT(puml, !IsPackage("library2"));

        REQUIRE_THAT(puml, IsDependency(_A("app"), _A("lib1")));
        REQUIRE_THAT(puml, IsDependency(_A("app"), _A("lib2")));
        REQUIRE_THAT(puml, IsDependency(_A("app"), _A("lib3")));
        REQUIRE_THAT(puml, IsDependency(_A("app"), _A("lib4")));
        //        REQUIRE_THAT(puml, IsDependency(_A("app"), _A("library1")));
        //        REQUIRE_THAT(puml, IsDependency(_A("app"), _A("library1")));

        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }
}