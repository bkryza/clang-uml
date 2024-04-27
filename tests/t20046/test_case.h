/**
 * tests/t20046/test_case.h
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t20046", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20046");

    auto diagram = config.diagrams["t20046_sequence"];

    REQUIRE(diagram->name == "t20046_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20046_sequence");

    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("tmain()::(lambda t20046.cc:13:15)"),
                "operator()()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20046.cc:13:15)"),
                _A("tmain()::(lambda t20046.cc:13:15)::(lambda "
                   "t20046.cc:14:16)"),
                "operator()()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20046.cc:13:15)::(lambda "
                       "t20046.cc:14:16)"),
                _A("a2(int)"), ""));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"),
                _A("a1<(lambda at t20046.cc:19:9)>((lambda at t20046.cc:19:9) "
                   "&&)"),
                ""));

        REQUIRE_THAT(src,
            HasCall(
                _A("a1<(lambda at t20046.cc:19:9)>((lambda at t20046.cc:19:9) "
                   "&&)"),
                _A("tmain()::(lambda t20046.cc:19:9)"), "operator()()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20046.cc:19:9)"),
                _A("tmain()::(lambda t20046.cc:19:9)::(lambda "
                   "t20046.cc:19:34)"),
                "operator()()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20046.cc:19:9)::(lambda "
                       "t20046.cc:19:34)"),
                _A("a3(int)"), ""));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsClass;

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}