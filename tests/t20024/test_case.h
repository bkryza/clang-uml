/**
 * tests/t20024/test_case.h
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

TEST_CASE("t20024")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20024", "t20024_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "A", "select(enum_a)"}, //
                {"A", "A", "a0()"},                 //
                {"A", "A", "a1()"},                 //
                {"A", "A", "a2()"},                 //
                {"A", "A", "a3()"},                 //

                {"tmain()", "B", "select(colors)"}, //
                {"B", "B", "red()"},                //
                {"B", "B", "orange()"},             //
                {"B", "B", "green()"},              //
                {"B", "B", "grey()"}                //
            }));
    });
    /*
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "select(enum_a)"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a0()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a1()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a2()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a3()"));

        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "select(colors)"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "red()"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "orange()"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "green()"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        std::vector<int> messages = {
            FindMessage(j, "tmain()", "A", "select(enum_a)"),
            FindMessage(j, "A", "A", "a0()"), FindMessage(j, "A", "A", "a1()"),
            FindMessage(j, "A", "A", "a2()"), FindMessage(j, "A", "A", "a3()")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;

        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "select(enum_a)"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a0()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a1()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a2()"));
        REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a3()"));

        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "select(colors)"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "red()"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "orange()"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("B"), "green()"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }*/
}