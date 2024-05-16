/**
 * tests/t20037/test_case.h
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

TEST_CASE("t20037")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20037", "t20037_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain(int,char **)", "a()", ""}, //
                {"a()", "A", "A()"},               //
                {"a()", "initb()", ""},            //
                {"a()", "B", "get()"},             //
                {"a()", "c()", ""},                //

//                {"tmain(int,char **)", "a()", ""}, //
//                {"a()", "B", "get()"},             //
//                {"a()", "c()", ""},                //
//
//                {"tmain(int,char **)", "a()", ""}, //
//                {"a()", "B", "get()"},             //
//                {"a()", "c()", ""}                 //
            }));
    });
    /*
        {
            auto src = generate_sequence_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            REQUIRE_THAT(src, HasCall(_A("tmain(int,char **)"), _A("a()"), ""));
            REQUIRE_THAT(src, HasCall(_A("a()"), _A("initb()"), ""));
            REQUIRE_THAT(src, HasCall(_A("a()"), _A("B"), "get()"));
            REQUIRE_THAT(src, HasCall(_A("a()"), _A("c()"), ""));

            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_sequence_json(diagram, *model);

            using namespace json;

            std::vector<int> messages = {
                FindMessage(j, "tmain(int,char **)", "a()", ""),
                FindMessage(j, "a()", "initb()", ""),
                FindMessage(j, "a()", "B", "get()"),
                FindMessage(j, "a()", "c()", "")};

            REQUIRE(std::is_sorted(messages.begin(), messages.end()));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }

        {
            auto src = generate_sequence_mermaid(diagram, *model);

            mermaid::SequenceDiagramAliasMatcher _A(src);
            using mermaid::HasCall;

            REQUIRE_THAT(src, HasCall(_A("tmain(int,char **)"), _A("a()"), ""));
            REQUIRE_THAT(src, HasCall(_A("a()"), _A("initb()"), ""));
            REQUIRE_THAT(src, HasCall(_A("a()"), _A("B"), "get()"));
            REQUIRE_THAT(src, HasCall(_A("a()"), _A("c()"), ""));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }*/
}