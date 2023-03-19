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

    const std::string expected_json = R"###(
{
  "diagram_type": "sequence",
  "name": "t20029_sequence",
  "participants": [
    {
      "id": "2091374738808319642",
      "name": "clanguml::t20029::tmain()",
      "source_location": {
        "file": "../../tests/t20029/t20029.cc",
        "line": 55
      },
      "type": "function"
    },
    {
      "id": "1673261195873192383",
      "name": "clanguml::t20029::Encoder<clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>>",
      "source_location": {
        "file": "../../tests/t20029/t20029.cc",
        "line": 11
      },
      "type": "class"
    },
    {
      "id": "658058855590948094",
      "name": "clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>",
      "source_location": {
        "file": "../../tests/t20029/t20029.cc",
        "line": 22
      },
      "type": "class"
    },
    {
      "id": "1896406205097618937",
      "name": "clanguml::t20029::ConnectionPool",
      "source_location": {
        "file": "../../tests/t20029/t20029.cc",
        "line": 39
      },
      "type": "class"
    },
    {
      "id": "1362646431260879440",
      "name": "clanguml::t20029::encode_b64(std::string &&)",
      "source_location": {
        "file": "../../tests/t20029/t20029.cc",
        "line": 9
      },
      "type": "function"
    }
  ],
  "sequences": [
    {
      "messages": [
        {
          "from": {
            "id": "2091374738808319642",
            "name": "clanguml::t20029::tmain()"
          },
          "name": "connect()",
          "return_type": "void",
          "scope": "normal",
          "source_location": {
            "file": "../../tests/t20029/t20029.cc",
            "line": 59
          },
          "to": {
            "activity_id": "940428568182104530",
            "activity_name": "clanguml::t20029::ConnectionPool::connect()",
            "participant_id": "1896406205097618937",
            "participant_name": "clanguml::t20029::ConnectionPool"
          },
          "type": "message"
        },
        {
          "activity_id": "2091374738808319642",
          "messages": [
            {
              "activity_id": "2091374738808319642",
              "branches": [
                {
                  "messages": [
                    {
                      "from": {
                        "id": "2091374738808319642",
                        "name": "clanguml::t20029::tmain()"
                      },
                      "name": "send(std::string &&)",
                      "return_type": "_Bool",
                      "scope": "condition",
                      "source_location": {
                        "file": "../../tests/t20029/t20029.cc",
                        "line": 62
                      },
                      "to": {
                        "activity_id": "2026763864005979273",
                        "activity_name": "clanguml::t20029::Encoder<clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>>::send(std::string &&)",
                        "participant_id": "1673261195873192383",
                        "participant_name": "clanguml::t20029::Encoder<clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>>"
                      },
                      "type": "message"
                    },
                    {
                      "from": {
                        "id": "2026763864005979273",
                        "name": "clanguml::t20029::Encoder<clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>>::send(std::string &&)"
                      },
                      "name": "encode(std::string &&)",
                      "return_type": "std::string",
                      "scope": "normal",
                      "source_location": {
                        "file": "../../tests/t20029/t20029.cc",
                        "line": 15
                      },
                      "to": {
                        "activity_id": "1468258269466480773",
                        "activity_name": "clanguml::t20029::Encoder<clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>>::encode(std::string &&)",
                        "participant_id": "1673261195873192383",
                        "participant_name": "clanguml::t20029::Encoder<clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>>"
                      },
                      "type": "message"
                    },
                    {
                      "from": {
                        "id": "1468258269466480773",
                        "name": "clanguml::t20029::Encoder<clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>>::encode(std::string &&)"
                      },
                      "name": "",
                      "return_type": "",
                      "scope": "normal",
                      "source_location": {
                        "file": "../../tests/t20029/t20029.cc",
                        "line": 19
                      },
                      "to": {
                        "activity_id": "1362646431260879440",
                        "activity_name": "clanguml::t20029::encode_b64(std::string &&)"
                      },
                      "type": "message"
                    },
                    {
                      "from": {
                        "id": "2026763864005979273",
                        "name": "clanguml::t20029::Encoder<clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>>::send(std::string &&)"
                      },
                      "name": "send(std::string &&)",
                      "return_type": "_Bool",
                      "scope": "normal",
                      "source_location": {
                        "file": "../../tests/t20029/t20029.cc",
                        "line": 15
                      },
                      "to": {
                        "activity_id": "30515971485361302",
                        "activity_name": "clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>::send(std::string &&)",
                        "participant_id": "658058855590948094",
                        "participant_name": "clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>"
                      },
                      "type": "message"
                    },
                    {
                      "activity_id": "30515971485361302",
                      "messages": [
                        {
                          "activity_id": "30515971485361302",
                          "branches": [
                            {
                              "messages": [
                                {
                                  "from": {
                                    "id": "30515971485361302",
                                    "name": "clanguml::t20029::Retrier<clanguml::t20029::ConnectionPool>::send(std::string &&)"
                                  },
                                  "name": "send(const std::string &)",
                                  "return_type": "_Bool",
                                  "scope": "condition",
                                  "source_location": {
                                    "file": "../../tests/t20029/t20029.cc",
                                    "line": 31
                                  },
                                  "to": {
                                    "activity_id": "972625940114169157",
                                    "activity_name": "clanguml::t20029::ConnectionPool::send(const std::string &)",
                                    "participant_id": "1896406205097618937",
                                    "participant_name": "clanguml::t20029::ConnectionPool"
                                  },
                                  "type": "message"
                                }
                              ],
                              "type": "consequent"
                            }
                          ],
                          "name": "if",
                          "type": "alt"
                        }
                      ],
                      "name": "while",
                      "type": "loop"
                    }
                  ],
                  "type": "consequent"
                }
              ],
              "name": "if",
              "type": "alt"
            }
          ],
          "name": "for",
          "type": "loop"
        }
      ],
      "start_from": {
        "id": 2091374738808319700,
        "location": "clanguml::t20029::tmain()"
      }
    }
  ],
  "using_namespace": "clanguml::t20029"
}
)###";

    auto j = generate_sequence_json(diagram, *model);

    REQUIRE(j == nlohmann::json::parse(expected_json));

    save_json(config.output_directory() + "/" + diagram->name + ".json", j);
}