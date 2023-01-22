/**
 * tests/t20029/test_case.h
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

TEST_CASE("t20029", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20029");

    auto diagram = config.diagrams["t20029_sequence"];

    REQUIRE(diagram->name == "t20029_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20029_sequence");

    auto puml = generate_sequence_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    // Check if all calls exist
    REQUIRE_THAT(
        puml, HasCall(_A("tmain()"), _A("ConnectionPool"), "connect()"));
    REQUIRE_THAT(puml,
        HasCallInControlCondition(_A("tmain()"),
            _A("Encoder<Retrier<ConnectionPool>>"), "send(std::string &&)"));

    REQUIRE_THAT(puml,
        HasCall(_A("Encoder<Retrier<ConnectionPool>>"),
            _A("Encoder<Retrier<ConnectionPool>>"), "encode(std::string &&)"));

    REQUIRE_THAT(puml,
        HasCall(_A("Encoder<Retrier<ConnectionPool>>"),
            _A("encode_b64(std::string &&)"), ""));

    REQUIRE_THAT(puml,
        HasCallInControlCondition(_A("Retrier<ConnectionPool>"),
            _A("ConnectionPool"), "send(const std::string &)"));

    REQUIRE_THAT(puml,
        !HasCall(_A("ConnectionPool"), _A("ConnectionPool"), "connect_impl()"));

    save_puml(config.output_directory() + "/" + diagram->name + ".puml", puml);
}