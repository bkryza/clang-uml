/**
 * tests/t20020/test_case.h
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

TEST_CASE("t20020", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20020");

    auto diagram = config.diagrams["t20020_sequence"];

    REQUIRE(diagram->name == "t20020_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20020_sequence");

    auto puml = generate_sequence_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    // Check if all calls exist
    REQUIRE_THAT(puml, HasCall(_A("tmain()"), _A("A"), "a1()"));
    REQUIRE_THAT(
        puml, HasCallInControlCondition(_A("tmain()"), _A("A"), "a2()"));
    REQUIRE_THAT(
        puml, HasCallInControlCondition(_A("tmain()"), _A("A"), "a3()"));

    REQUIRE_THAT(puml, HasCall(_A("tmain()"), _A("B"), "b1()"));
    REQUIRE_THAT(puml, HasCall(_A("tmain()"), _A("B"), "b2()"));
    REQUIRE_THAT(puml, HasCall(_A("tmain()"), _A("B"), "log()"));

    REQUIRE_THAT(puml, HasCall(_A("tmain()"), _A("C"), "c1()"));
    REQUIRE_THAT(puml, HasCallInControlCondition(_A("C"), _A("C"), "c2()"));
    REQUIRE_THAT(
        puml, HasCallInControlCondition(_A("tmain()"), _A("C"), "c3(int)"));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}