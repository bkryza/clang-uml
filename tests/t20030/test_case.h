/**
 * tests/t20030/test_case.h
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

TEST_CASE("t20030")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20030", "t20030_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain(int)", "magic()", ""},          //
                {"tmain(int)", "A", "A(int)"},          //
                {"tmain(int)", "A", "operator+=(int)"}, //
                {"A", "A", "add(int)"},                 //
                                {"tmain(bool,int)", "A", "A()"}, //
                //                {"A", "A", "create()"},             //
                //                {"tmain(bool,int)", "A", "A()"}, //
                //                {"A", "A", "create()"},             //
                //                {"tmain(bool,int)", "A", "operator+=(int)"},
                //                //
                //                {"A", "A", "add(int)"}, //
                //                {"tmain(bool,int)", "A", "operator=(const A
                //                &)"}, //
                //                {"A", "A", "set(int)"}, //
                // {"tmain(bool,int)", "A", "value() const"}         //
            }));
    });
    /*
        {
            auto src = generate_sequence_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            // Check if all calls exist
            REQUIRE_THAT(src, HasCall(_A("tmain(int)"), _A("magic()"), ""));

            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "create()"));
            REQUIRE_THAT(
                src, HasCall(_A("tmain(int)"), _A("A"), "operator+=(int)"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "add(int)"));

            REQUIRE_THAT(src, HasCall(_A("tmain(bool,int)"), _A("A"), "A()"));
            REQUIRE_THAT(
                src, HasCall(_A("tmain(bool,int)"), _A("A"),
       "operator+=(int)")); REQUIRE_THAT(src, HasCall(_A("A"), _A("A"),
       "add(int)")); REQUIRE_THAT(src, HasCall(_A("tmain(bool,int)"), _A("A"),
       "operator=(const A &)")); REQUIRE_THAT(src, HasCall(_A("A"), _A("A"),
       "set(int)")); REQUIRE_THAT(src, HasCall(_A("tmain(bool,int)"), _A("A"),
       "value()"));

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

            REQUIRE_THAT(src, HasCall(_A("tmain(int)"), _A("magic()"), ""));

            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "create()"));
            REQUIRE_THAT(
                src, HasCall(_A("tmain(int)"), _A("A"), "operator+=(int)"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "add(int)"));

            REQUIRE_THAT(src, HasCall(_A("tmain(bool,int)"), _A("A"), "A()"));
            REQUIRE_THAT(
                src, HasCall(_A("tmain(bool,int)"), _A("A"),
       "operator+=(int)")); REQUIRE_THAT(src, HasCall(_A("A"), _A("A"),
       "add(int)")); REQUIRE_THAT(src, HasCall(_A("tmain(bool,int)"), _A("A"),
       "operator=(const A &)")); REQUIRE_THAT(src, HasCall(_A("A"), _A("A"),
       "set(int)")); REQUIRE_THAT(src, HasCall(_A("tmain(bool,int)"), _A("A"),
       "value()"));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }*/
}