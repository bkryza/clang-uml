/**
 * tests/t40003/test_case.cc
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

TEST_CASE("t40003", "[test-case][include]")
{
    auto [config, db] = load_config("t40003");

    auto diagram = config.diagrams["t40003_include"];

    REQUIRE(diagram->name == "t40003_include");

    auto model = generate_include_diagram(*db, diagram);

    REQUIRE(model->name() == "t40003_include");

    auto puml = generate_include_puml(diagram, *model);

    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, IsFolder("dependants"));
    REQUIRE_THAT(puml, IsFolder("dependencies"));

    REQUIRE_THAT(puml, IsFile("t1.h"));
    REQUIRE_THAT(puml, IsFile("t2.h"));
    REQUIRE_THAT(puml, IsFile("t3.h"));

    REQUIRE_THAT(puml, !IsFile("t4.h"));
    REQUIRE_THAT(puml, IsFile("t5.h"));
    REQUIRE_THAT(puml, !IsFile("t6.h"));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
