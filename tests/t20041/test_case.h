/**
 * tests/t20041/test_case.h
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

TEST_CASE("t20041")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20041", "t20041_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "A<int,double,std::string>",
                    "print(int,double,std::string)"}, //
                {"A<int,double,std::string>", "A<double,std::string>",
                    "print(double,std::string)"}, //
                {"A<double,std::string>", "A<std::string>",
                    "print(std::string)"},          //
                {"A<std::string>", "A", "print()"}, //
            }));
    });
    /*
        {
            auto src = generate_sequence_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            REQUIRE_THAT(src,
                HasCall(_A("tmain()"), _A("A<int,double,std::string>"),
                    "print(int,double,std::string)"));
            REQUIRE_THAT(src,
                HasCall(_A("A<int,double,std::string>"),
                    _A("A<double,std::string>"), "print(double,std::string)"));
            REQUIRE_THAT(src,
                HasCall(_A("A<double,std::string>"), _A("A<std::string>"),
                    "print(std::string)"));
            REQUIRE_THAT(src, HasCall(_A("A<std::string>"), _A("A"),
       "print()"));

            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_sequence_json(diagram, *model);

            using namespace json;

            std::vector<int> messages = {
                FindMessage(j, "tmain()", "A<int,double,std::string>",
                    "print(int,double,std::string)"),
                FindMessage(j, "A<int,double,std::string>",
       "A<double,std::string>", "print(double,std::string)"), FindMessage(j,
       "A<double,std::string>", "A<std::string>", "print(std::string)"),
                FindMessage(j, "A<std::string>", "A", "print()")};

            REQUIRE(std::is_sorted(messages.begin(), messages.end()));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }

        {
            auto src = generate_sequence_mermaid(diagram, *model);

            mermaid::SequenceDiagramAliasMatcher _A(src);
            using mermaid::HasCall;

            REQUIRE_THAT(src,
                HasCall(_A("tmain()"), _A("A<int,double,std::string>"),
                    "print(int,double,std::string)"));
            REQUIRE_THAT(src,
                HasCall(_A("A<int,double,std::string>"),
                    _A("A<double,std::string>"), "print(double,std::string)"));
            REQUIRE_THAT(src,
                HasCall(_A("A<double,std::string>"), _A("A<std::string>"),
                    "print(std::string)"));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }*/
}