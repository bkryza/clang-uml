/**
 * tests/t20018/test_case.h
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

TEST_CASE("t20018")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20018", "t20018_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "Answer<Factorial<5>,120>", "print()", Static{}}, //
                {"Answer<Factorial<5>,120>", "Factorial<5>", "print(int)",
                    Static{}},                                            //
                {"Factorial<5>", "Factorial<4>", "print(int)", Static{}}, //
                {"Factorial<4>", "Factorial<3>", "print(int)", Static{}}, //
                {"Factorial<3>", "Factorial<2>", "print(int)", Static{}}, //
                {"Factorial<2>", "Factorial<1>", "print(int)", Static{}}, //
                {"Factorial<1>", "Factorial<0>", "print(int)", Static{}}, //
            }));
    });
    /*
        {
            auto src = generate_sequence_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            // Check if all calls exist
            REQUIRE_THAT(src,
                HasCall(
                    _A("tmain()"), _A("Answer<Factorial<5>,120>"),
       "__print()__")); REQUIRE_THAT(src,
                HasCall(_A("Answer<Factorial<5>,120>"), _A("Factorial<5>"),
                    "__print(int)__"));
            REQUIRE_THAT(src,
                HasCall(_A("Factorial<5>"), _A("Factorial<4>"),
       "__print(int)__")); REQUIRE_THAT(src, HasCall(_A("Factorial<4>"),
       _A("Factorial<3>"), "__print(int)__")); REQUIRE_THAT(src,
                HasCall(_A("Factorial<3>"), _A("Factorial<2>"),
       "__print(int)__")); REQUIRE_THAT(src, HasCall(_A("Factorial<2>"),
       _A("Factorial<1>"), "__print(int)__")); REQUIRE_THAT(src,
                HasCall(_A("Factorial<1>"), _A("Factorial<0>"),
       "__print(int)__"));

            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_sequence_json(diagram, *model);

            using namespace json;

            std::vector<int> messages = {
                FindMessage(j, "tmain()", "Answer<Factorial<5>,120>",
       "print()"), FindMessage( j, "Answer<Factorial<5>,120>", "Factorial<5>",
       "print(int)"), FindMessage(j, "Factorial<5>", "Factorial<4>",
       "print(int)"), FindMessage(j, "Factorial<4>", "Factorial<3>",
       "print(int)"), FindMessage(j, "Factorial<3>", "Factorial<2>",
       "print(int)"), FindMessage(j, "Factorial<2>", "Factorial<1>",
       "print(int)"), FindMessage(j, "Factorial<1>", "Factorial<0>",
       "print(int)")};

            REQUIRE(std::is_sorted(messages.begin(), messages.end()));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }

        {
            auto src = generate_sequence_mermaid(diagram, *model);

            mermaid::SequenceDiagramAliasMatcher _A(src);
            using mermaid::HasCall;

            REQUIRE_THAT(src,
                HasCall(_A("tmain()"), _A("Answer<Factorial<5>,120>"),
       "print()")); REQUIRE_THAT(src, HasCall(_A("Answer<Factorial<5>,120>"),
       _A("Factorial<5>"), "print(int)")); REQUIRE_THAT( src,
       HasCall(_A("Factorial<5>"), _A("Factorial<4>"), "print(int)"));
            REQUIRE_THAT(
                src, HasCall(_A("Factorial<4>"), _A("Factorial<3>"),
       "print(int)")); REQUIRE_THAT( src, HasCall(_A("Factorial<3>"),
       _A("Factorial<2>"), "print(int)")); REQUIRE_THAT( src,
       HasCall(_A("Factorial<2>"), _A("Factorial<1>"), "print(int)"));
            REQUIRE_THAT(
                src, HasCall(_A("Factorial<1>"), _A("Factorial<0>"),
       "print(int)"));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }
        */
}