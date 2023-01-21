/**
 * tests/t20009/test_case.h
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

TEST_CASE("t20009", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20009");

    auto diagram = config.diagrams["t20009_sequence"];

    REQUIRE(diagram->name == "t20009_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20009_sequence");

    auto puml = generate_sequence_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    // Check if all calls exist
    REQUIRE_THAT(
        puml, HasCall(_A("tmain()"), _A("B<std::string>"), "b(std::string)"));
    REQUIRE_THAT(puml,
        HasCall(_A("B<std::string>"), _A("A<std::string>"), "a(std::string)"));

    REQUIRE_THAT(puml, HasCall(_A("tmain()"), _A("B<int>"), "b(int)"));
    REQUIRE_THAT(puml, HasCall(_A("B<int>"), _A("A<int>"), "a(int)"));

    REQUIRE_THAT(puml, HasCall(_A("tmain()"), _A("B<float>"), "b(float)"));
    REQUIRE_THAT(puml, HasCall(_A("B<float>"), _A("A<float>"), "a(float)"));
    save_puml(config.output_directory() + "/" + diagram->name + ".puml", puml);
}