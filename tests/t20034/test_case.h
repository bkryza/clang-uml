/**
 * tests/t20034/test_case.h
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

TEST_CASE("t20034")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20034", "t20034_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageChainsOrder(src,
            {
                //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "C", "c2()"},          //
                    {"C", "B", "b2()"},          //
                    {"B", "A", "a2()"}           //
                },                               //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "D::d2()::(lambda t20034.cc:56:18)",
                        "operator()() const"},                         //
                    {"D::d2()::(lambda t20034.cc:56:18)", "A", "a2()"} //
                },                                                     //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "A", "a2()"}           //
                },                               //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "C", "c4()"},          //
                    {"C", "B", "b4()"},          //
                    {"B", "B", "b2()"},          //
                    {"B", "A", "a2()"}           //
                },                               //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "C", "c1()"},          //
                    {"C", "B", "b1()"},          //
                    {"B", "A", "a2()"}           //
                },                               //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "C", "c3()"},          //
                    {"C", "C", "c2()"},          //
                    {"C", "B", "b2()"},          //
                    {"B", "A", "a2()"}           //
                } //
            }));
    });
    /*
        {
            auto src = generate_sequence_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            // Check if all calls exist
            REQUIRE_THAT(src, HasCall(_A("D"), _A("A"), "a2()"));

            REQUIRE_THAT(src, HasCall(_A("D"), _A("C"), "c3()"));
            REQUIRE_THAT(src, HasCall(_A("C"), _A("C"), "c2()"));
            REQUIRE_THAT(src, HasCall(_A("C"), _A("B"), "b2()"));
            REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "a2()"));

            REQUIRE_THAT(src, HasCall(_A("D"), _A("C"), "c4()"));

            REQUIRE_THAT(src, !HasCall(_A("C"), _A("B"), "b3()"));

            save_puml(config.output_directory(), diagram->name +
       ".puml", src);
        }

        {
            auto j = generate_sequence_json(diagram, *model);

            using namespace json;

            REQUIRE(HasMessageChain(j,
                {{"d2()", "c3()", "void"}, {"c3()", "c2()", "void"},
                    {"c2()", "b2()", "void"}, {"b2()", "a2()",
       "void"}})); REQUIRE(HasMessageChain(j,
                {{"d2()", "c4()", "void"}, {"c4()", "b4()", "void"},
                    {"b4()", "b2()", "void"}, {"b2()", "a2()",
       "void"}})); REQUIRE(HasMessageChain(j, {{"d2()", "a2()",
       "void"}})); REQUIRE(HasMessageChain(j,
                {{"d2()", "c1()", "void"}, {"c1()", "b1()", "void"},
                    {"b1()", "a2()", "void"}}));
            REQUIRE(HasMessageChain(j,
                {{"d2()", "c2()", "void"}, {"c2()", "b2()", "void"},
                    {"b2()", "a2()", "void"}}));

            save_json(config.output_directory(), diagram->name +
       ".json", j);
        }

        {
            auto src = generate_sequence_mermaid(diagram, *model);

            mermaid::SequenceDiagramAliasMatcher _A(src);
            using mermaid::HasCall;

            REQUIRE_THAT(src, HasCall(_A("D"), _A("A"), "a2()"));

            REQUIRE_THAT(src, HasCall(_A("D"), _A("C"), "c3()"));
            REQUIRE_THAT(src, HasCall(_A("C"), _A("C"), "c2()"));
            REQUIRE_THAT(src, HasCall(_A("C"), _A("B"), "b2()"));
            REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "a2()"));

            REQUIRE_THAT(src, HasCall(_A("D"), _A("C"), "c4()"));

            REQUIRE_THAT(src, !HasCall(_A("C"), _A("B"), "b3()"));

            save_mermaid(config.output_directory(), diagram->name +
       ".mmd", src);
        }
        */
}