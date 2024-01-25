/**
 * test_case.h
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

TEST_CASE("t20012", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20012");

    auto diagram = config.diagrams["t20012_sequence"];

    REQUIRE(diagram->name == "t20012_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20012_sequence");
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("tmain()::(lambda t20012.cc:67:20)"),
                "operator()()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:67:20)"), _A("A"), "a()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "aa()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "aaa()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:67:20)"), _A("B"), "b()"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "bb()"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "bbb()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:80:20)"), _A("C"), "c()"));
        REQUIRE_THAT(src, HasCall(_A("C"), _A("C"), "cc()"));
        REQUIRE_THAT(src, HasCall(_A("C"), _A("C"), "ccc()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:80:20)"),
                _A("tmain()::(lambda t20012.cc:67:20)"), "operator()()"));

        REQUIRE_THAT(src, HasCall(_A("C"), _A("C"), "ccc()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("R<(lambda at t20012.cc:86:9)>"),
                "R((lambda at t20012.cc:86:9) &&)"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("R<(lambda at t20012.cc:86:9)>"), "r()"));
        REQUIRE_THAT(src,
            HasCall(_A("R<(lambda at t20012.cc:86:9)>"),
                _A("tmain()::(lambda t20012.cc:86:9)"), "operator()()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:86:9)"), _A("C"), "c()"));

        // @todo #168
        // REQUIRE_THAT(puml, HasCall(_A("tmain()"), _A("D"), "add5(int)"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        std::vector<int> messages = {
            FindMessage(j, "tmain()", "tmain()::(lambda t20012.cc:67:20)",
                "operator()()"),
            FindMessage(j, "tmain()::(lambda t20012.cc:67:20)", "A", "a()"),
            FindMessage(j, "A", "A", "aa()"), FindMessage(j, "A", "A", "aaa()"),
            FindMessage(j, "tmain()::(lambda t20012.cc:67:20)", "B", "b()"),
            FindMessage(j, "B", "B", "bb()"), FindMessage(j, "B", "B", "bbb()"),
            FindMessage(j, "tmain()::(lambda t20012.cc:80:20)", "C", "c()"),
            FindMessage(j, "C", "C", "cc()"), FindMessage(j, "C", "C", "ccc()"),
            FindMessage(j, "tmain()::(lambda t20012.cc:80:20)",
                "tmain()::(lambda t20012.cc:67:20)", "operator()()"),
            FindMessage(j, "tmain()", "R<(lambda at t20012.cc:86:9)>", "r()"),
            FindMessage(j, "R<(lambda at t20012.cc:86:9)>",
                "tmain()::(lambda t20012.cc:86:9)", "operator()()"),
            FindMessage(j, "tmain()::(lambda t20012.cc:86:9)", "C", "c()"),
            // @todo #168
            // FindMessage(j, "tmain()", "D", "add5(int)")
        };

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("tmain()::(lambda t20012.cc:67:20)"),
                "operator()()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:67:20)"), _A("A"), "a()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "aa()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "aaa()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:67:20)"), _A("B"), "b()"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "bb()"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "bbb()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:80:20)"), _A("C"), "c()"));
        REQUIRE_THAT(src, HasCall(_A("C"), _A("C"), "cc()"));
        REQUIRE_THAT(src, HasCall(_A("C"), _A("C"), "ccc()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:80:20)"),
                _A("tmain()::(lambda t20012.cc:67:20)"), "operator()()"));

        REQUIRE_THAT(src, HasCall(_A("C"), _A("C"), "ccc()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("R<(lambda at t20012.cc:86:9)>"), "r()"));
        REQUIRE_THAT(src,
            HasCall(_A("R<(lambda at t20012.cc:86:9)>"),
                _A("tmain()::(lambda t20012.cc:86:9)"), "operator()()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20012.cc:86:9)"), _A("C"), "c()"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}
