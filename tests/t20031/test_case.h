/**
 * tests/t20031/test_case.h
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

TEST_CASE("t20031", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20031");

    auto diagram = config.diagrams["t20031_sequence"];

    REQUIRE(diagram->name == "t20031_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20031_sequence");

    {
        auto puml = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        REQUIRE_THAT(puml, HasCall(_A("tmain(int)"), _A("magic()"), ""));

        REQUIRE_THAT(puml, !HasCall(_A("A"), _A("A"), "create()"));
        REQUIRE_THAT(
            puml, !HasCall(_A("tmain(int)"), _A("A"), "operator+=(int)"));
        REQUIRE_THAT(puml, !HasCall(_A("A"), _A("A"), "add(int)"));

        REQUIRE_THAT(puml, !HasCall(_A("tmain(bool,int)"), _A("A"), "A()"));
        REQUIRE_THAT(
            puml, !HasCall(_A("tmain(bool,int)"), _A("A"), "operator+=(int)"));
        REQUIRE_THAT(puml, !HasCall(_A("A"), _A("A"), "add(int)"));
        REQUIRE_THAT(puml,
            !HasCall(_A("tmain(bool,int)"), _A("A"), "operator=(const A &)"));
        REQUIRE_THAT(puml, !HasCall(_A("A"), _A("A"), "set(int)"));
        REQUIRE_THAT(puml, HasCall(_A("tmain(bool,int)"), _A("A"), "value()"));
        REQUIRE_THAT(puml,
            !HasCall(_A("tmain(bool,int)::(lambda "
                        "../../tests/t20031/t20031.cc:47:26)"),
                _A("zero()"), ""));

        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }
}