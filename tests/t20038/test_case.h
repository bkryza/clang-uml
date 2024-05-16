/**
 * tests/t20038/test_case.h
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

TEST_CASE("t20038")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20038", "t20038_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "B", "b()"}, //
                {"B", "A", "a()"},       //
                {"tmain()", "B", "b()"}, //
                {"B", "A", "a()"},       //

                {"tmain()", "B", "bbb()"}, //
                {"B", "A", "aaa()"},       //

                {"tmain()", "B", "bbbb()"},                          //
                {"B", "A", "aaaa()"},                                //
                {"A", "add<int>(int,int)", ""},                      //
                {"add<int>(int,int)", "add_impl<int>(int,int)", ""}, //

                {"tmain()", "B", "wrap(int)"}, //

                {"tmain()", "add_impl<double>(double,double)", ""}, //

                {"tmain()", "B", "bbbbb()"},                         //
                {"B", "A", "aaaa()"},                                //
                {"A", "add<int>(int,int)", ""},                      //
                {"add<int>(int,int)", "add_impl<int>(int,int)", ""}, //

                {"tmain()", "B", "bbb()", InControlCondition{}}, //
                {"B", "A", "aaa()"},                             //
            }));
    });
    /*
        {
            auto src = generate_sequence_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "b()"));

            REQUIRE_THAT(src, !HasCall(_A("tmain()"), _A("B"), "bb()"));

            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "bbb()"));

            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "bbbb()"));

            REQUIRE_THAT(src,
                HasMessageComment(_A("tmain()"),
                    "This comment should be rendered only\\n"
                    "once"));

            REQUIRE_THAT(src,
                HasCall(_A("tmain()"), _A("add_impl<double>(double,double)"),
       ""));

            REQUIRE_THAT(
                src, HasMessageComment(_A("tmain()"), "What is 2 \\+ 2\\?"));

            REQUIRE_THAT(src,
                !HasMessageComment(
                    _A("tmain()"), "This is specific for some_other_diagram"));

            REQUIRE_THAT(
                src, HasMessageComment(_A("tmain()"), "Calling
       B::bbbbb\\(\\)"));

            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "bbbbb()"));

            REQUIRE_THAT(src,
                HasMessageComment(_A("tmain()"), "This is a conditional
       operator"));

            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_sequence_json(diagram, *model);

            using namespace json;

            save_json(config.output_directory(), diagram->name + ".json", j);
        }

        {
            auto src = generate_sequence_mermaid(diagram, *model);

            mermaid::SequenceDiagramAliasMatcher _A(src);
            using mermaid::HasCall;
            using mermaid::HasCallInControlCondition;
            using mermaid::HasMessageComment;

            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "b()"));

            REQUIRE_THAT(src, !HasCall(_A("tmain()"), _A("B"), "bb()"));

            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "bbb()"));

            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "bbbb()"));

            REQUIRE_THAT(src,
                HasMessageComment(_A("tmain()"),
                    "This comment should be rendered only<br/>"
                    "once"));

            REQUIRE_THAT(src,
                HasCall(_A("tmain()"), _A("add_impl<double>(double,double)"),
       ""));

            REQUIRE_THAT(
                src, HasMessageComment(_A("tmain()"), "What is 2 \\+ 2\\?"));

            REQUIRE_THAT(src,
                !HasMessageComment(
                    _A("tmain()"), "This is specific for some_other_diagram"));

            REQUIRE_THAT(
                src, HasMessageComment(_A("tmain()"), "Calling
       B::bbbbb\\(\\)"));

            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B"), "bbbbb()"));

            REQUIRE_THAT(src,
                !HasMessageComment(
                    _A("tmain()"), "This is specific for some_other_diagram"));

            REQUIRE_THAT(
                src, HasMessageComment(_A("tmain()"), "Calling
       B::bbbbb\\(\\)"));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }
        */
}