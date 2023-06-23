/**
 * tests/t20006/test_case.h
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

TEST_CASE("t20006", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20006");

    auto diagram = config.diagrams["t20006_sequence"];

    REQUIRE(diagram->name == "t20006_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20006_sequence");

    {
        auto puml = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(puml, HasCall(_A("tmain()"), _A("B<int>"), "b(int)"));
        REQUIRE_THAT(puml, HasCall(_A("B<int>"), _A("A<int>"), "a1(int)"));

        REQUIRE_THAT(puml,
            HasCall(_A("tmain()"), _A("B<std::string>"), "b(std::string)"));
        REQUIRE_THAT(puml,
            HasCall(
                _A("B<std::string>"), _A("A<std::string>"), "a2(std::string)"));

        REQUIRE_THAT(
            puml, HasCall(_A("tmain()"), _A("BB<int,int>"), "bb1(int,int)"));
        REQUIRE_THAT(
            puml, HasCall(_A("BB<int,int>"), _A("AA<int>"), "aa1(int)"));

        REQUIRE_THAT(
            puml, HasCall(_A("tmain()"), _A("BB<int,int>"), "bb2(int,int)"));
        REQUIRE_THAT(
            puml, HasCall(_A("BB<int,int>"), _A("AA<int>"), "aa2(int)"));

        REQUIRE_THAT(puml,
            HasCall(_A("tmain()"), _A("BB<int,std::string>"),
                "bb1(int,std::string)"));
        REQUIRE_THAT(puml,
            HasCall(_A("BB<int,std::string>"), _A("AA<int>"), "aa2(int)"));

        REQUIRE_THAT(puml,
            HasCall(_A("tmain()"), _A("BB<int,std::string>"),
                "bb2(int,std::string)"));
        REQUIRE_THAT(puml,
            HasCall(_A("BB<int,std::string>"), _A("AA<int>"), "aa1(int)"));

        REQUIRE_THAT(puml,
            HasCall(_A("tmain()"), _A("BB<int,float>"), "bb1(int,float)"));
        REQUIRE_THAT(puml,
            HasCall(
                _A("BB<int,float>"), _A("BB<int,float>"), "bb2(int,float)"));
        REQUIRE_THAT(
            puml, HasCall(_A("BB<int,float>"), _A("AA<int>"), "aa2(int)"));

        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        std::vector<int> messages = {
            FindMessage(j, "tmain()", "B<int>", "b(int)"),
            FindMessage(j, "B<int>", "A<int>", "a1(int)"),
            FindMessage(j, "tmain()", "B<std::string>", "b(std::string)"),
            FindMessage(j, "tmain()", "BB<int,int>", "bb1(int,int)"),
            FindMessage(j, "BB<int,int>", "AA<int>", "aa1(int)"),
            FindMessage(
                j, "tmain()", "BB<int,std::string>", "bb1(int,std::string)")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }
}