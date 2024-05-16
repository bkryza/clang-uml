/**
 * tests/t20035/test_case.h
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

TEST_CASE("t20035")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20035", "t20035_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageChainsOrder(src,
            {{
                //
                {"tmain(int,char **)", "a(int)", ""}, //
                {"a(int)", "b1(int)", ""},            //
                {"b1(int)", "c(int)", ""}             //
            }}));
    });
    /*
        {
            auto src = generate_sequence_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            REQUIRE_THAT(src, HasCall(_A("tmain(int,char **)"), _A("a(int)"),
       "")); REQUIRE_THAT(src, HasCall(_A("a(int)"), _A("b1(int)"), ""));
            REQUIRE_THAT(src, HasCall(_A("b1(int)"), _A("c(int)"), ""));

            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_sequence_json(diagram, *model);

            using namespace json;

            REQUIRE(HasMessageChain(j,
                {{"tmain(int,char **)", "a(int)", "int"},
                    {"a(int)", "b1(int)", "int"}, {"b1(int)", "c(int)",
       "int"}}));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }

        {
            auto src = generate_sequence_mermaid(diagram, *model);

            mermaid::SequenceDiagramAliasMatcher _A(src);
            using mermaid::HasCall;

            REQUIRE_THAT(src, HasCall(_A("tmain(int,char **)"), _A("a(int)"),
       "")); REQUIRE_THAT(src, HasCall(_A("a(int)"), _A("b1(int)"), ""));
            REQUIRE_THAT(src, HasCall(_A("b1(int)"), _A("c(int)"), ""));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }
        */
}