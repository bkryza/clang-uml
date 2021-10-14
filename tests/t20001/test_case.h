/**
 * tests/t20001/test_case.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t20001"}));

    REQUIRE(diagram->exclude.namespaces.size() == 1);
    REQUIRE_THAT(diagram->exclude.namespaces,
        VectorContains(std::string{"clanguml::t20001::detail"}));

    REQUIRE(diagram->should_include("clanguml::t20001::A"));
    REQUIRE(!diagram->should_include("clanguml::t20001::detail::C"));
    REQUIRE(!diagram->should_include("std::vector"));

    REQUIRE(diagram->name == "t20001_sequence");

    auto model = generate_sequence_diagram(db, diagram);

    REQUIRE(model.name == "t20001_sequence");

    auto puml = generate_sequence_puml(diagram, model);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, HasCall("A", "log_result"));
    REQUIRE_THAT(puml, HasCall("B", "A", "log_result"));
    REQUIRE_THAT(puml, HasCallWithResponse("B", "A", "add3"));
    REQUIRE_THAT(puml, HasCall("A", "add"));
    REQUIRE_THAT(puml, !HasCall("A", "detail::C", "add"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
