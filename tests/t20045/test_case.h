/**
 * tests/t20045/test_case.h
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

TEST_CASE("t20045", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20045");

    auto diagram = config.diagrams["t20045_sequence"];

    REQUIRE(diagram->name == "t20045_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20045_sequence");

    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("a2(int)"), ""));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"),
                _A("a1<(lambda at t20045.cc:35:18)>((lambda at "
                   "t20045.cc:35:18) &&)"),
                ""));

        REQUIRE_THAT(src,
            HasCall(_A("a1<(lambda at t20045.cc:35:18)>((lambda at "
                       "t20045.cc:35:18) &&)"),
                _A("tmain()::(lambda t20045.cc:35:18)"), "operator()()"));

        REQUIRE_THAT(src,
            HasCall(
                _A("tmain()::(lambda t20045.cc:35:18)"), _A("a3(int)"), ""));

        REQUIRE_THAT(src,
            HasCall(
                _A("tmain()::(lambda t20045.cc:37:18)"), _A("B"), "b1(int)"));

        REQUIRE_THAT(src,
            HasCall(
                _A("tmain()::(lambda t20045.cc:39:18)"), _A("C"), "get_x()"));

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