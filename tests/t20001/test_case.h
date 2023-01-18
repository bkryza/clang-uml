/**
 * tests/t20001/test_case.cc
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

TEST_CASE("t20001", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20001");

    auto diagram = config.diagrams["t20001_sequence"];

    REQUIRE(diagram->name == "t20001_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20001_sequence");

    REQUIRE(model->should_include("clanguml::t20001::A"));
    REQUIRE(!model->should_include("clanguml::t20001::detail::C"));
    REQUIRE(!model->should_include("std::vector"));

    auto puml = generate_sequence_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, HasCall(_A("B"), _A("A"), "add3(int,int,int)"));
    REQUIRE_THAT(puml, HasCall(_A("A"), "add(int,int)"));
    REQUIRE_THAT(puml, !HasCall(_A("A"), _A("detail::C"), "add(int,int)"));
    REQUIRE_THAT(puml, HasCall(_A("A"), "__log_result(int)__"));
    REQUIRE_THAT(puml, HasCall(_A("B"), _A("A"), "__log_result(int)__"));

    REQUIRE_THAT(puml, HasComment("t20001 test diagram of type sequence"));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
