/**
 * tests/t20017/test_case.h
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

TEST_CASE("t20017", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20017");

    auto diagram = config.diagrams["t20017_sequence"];

    REQUIRE(diagram->name == "t20017_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20017_sequence");
    {
        auto puml = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(puml, HasEntrypoint(_A("t20017.cc"), "tmain()"));
        REQUIRE_THAT(puml,
            HasCall(_A("t20017.cc"), _A("include/t20017_a.h"), "a1(int,int)"));
        REQUIRE_THAT(puml,
            HasCall(_A("t20017.cc"), _A("include/t20017_a.h"), "a2(int,int)"));
        REQUIRE_THAT(puml,
            HasCall(_A("t20017.cc"), _A("include/t20017_a.h"), "a3(int,int)"));
        REQUIRE_THAT(puml,
            HasCall(_A("t20017.cc"), _A("include/t20017_b.h"), "b1(int,int)"));
        REQUIRE_THAT(puml,
            HasCall(
                _A("t20017.cc"), _A("include/t20017_b.h"), "b2<int>(int,int)"));
        REQUIRE_THAT(puml, HasExitpoint(_A("t20017.cc")));

        save_puml(config.output_directory(), diagram->name + ".puml", puml);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        std::vector<int> messages = {
            FindMessage(j, File("t20017.cc"), File("include/t20017_a.h"),
                "a3(int,int)"),
            FindMessage(j, File("t20017.cc"), File("include/t20017_b.h"),
                "b1(int,int)"),
            FindMessage(j, File("t20017.cc"), File("include/t20017_a.h"),
                "a2(int,int)"),
            FindMessage(j, File("t20017.cc"), File("include/t20017_a.h"),
                "a1(int,int)"),
            FindMessage(j, File("t20017.cc"), File("include/t20017_b.h"),
                "b2<int>(int,int)")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto mmd = generate_sequence_mermaid(diagram, *model);

        save_mermaid(config.output_directory(), diagram->name + ".mmd", mmd);
    }
}