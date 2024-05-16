/**
 * tests/t20001/test_case.h
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

TEST_CASE("t20001")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20001", "t20001_sequence");

    CHECK_SEQUENCE_DIAGRAM(
        config, diagram, *model,
        [](const auto &src) {
            REQUIRE(HasTitle(src, "Basic sequence diagram example"));

            REQUIRE(MessageOrder(src,
                {
                    //
                    {"tmain()", "A", "A()"}, //
                    {"tmain()", "B", "B(A &)"}, //

                    {"tmain()", "A", "add(int,int)"},           //

                    {"tmain()", "B", "wrap_add3(int,int,int)"}, //
                    {"B", "A", "add3(int,int,int)"},            //
                    {"A", "A", "add(int,int)"},                 //
                    {"A", "A", "log_result(int)", Static{}},    //
                    {"B", "A", "log_result(int)", Static{}}     //
                }));

            REQUIRE(!HasMessage(src, {"A", {"detail", "C"}, "add(int,int)"}));

            REQUIRE(HasComment(src, "t20001 test diagram of type sequence"));

            REQUIRE(HasMessageComment(src, "tmain()", "Just add 2 numbers"));

            REQUIRE(HasMessageComment(src, "tmain()", "And now add another 2"));
        },
        [](const json_t &src) {
            const auto &A = get_participant(src.src, "A");

            CHECK(A.has_value());

            CHECK(A.value()["type"] == "class");
            CHECK(A.value()["name"] == "A");
            CHECK(A.value()["display_name"] == "A");
            CHECK(A.value()["namespace"] == "clanguml::t20001");
            CHECK(A.value()["source_location"]["file"] == "t20001.cc");
            CHECK(A.value()["source_location"]["line"] == 13);

            const auto &tmain = get_participant(src.src, "tmain()");

            CHECK(tmain.has_value());

            CHECK(tmain.value()["type"] == "function");
            CHECK(tmain.value()["name"] == "tmain");
            CHECK(tmain.value()["display_name"] == "tmain()");
            CHECK(tmain.value()["namespace"] == "clanguml::t20001");
            CHECK(tmain.value()["source_location"]["file"] == "t20001.cc");
            CHECK(tmain.value()["source_location"]["line"] == 61);
        });

    /*
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, HasTitle("Basic sequence diagram example"));

        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "add3(int,int,int)"));
        REQUIRE_THAT(src, HasCall(_A("A"), "add(int,int)"));
        REQUIRE_THAT(src, !HasCall(_A("A"), _A("detail::C"), "add(int,int)"));
        REQUIRE_THAT(src, HasCall(_A("A"), "__log_result(int)__"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "__log_result(int)__"));

        REQUIRE_THAT(src, HasComment("t20001 test diagram of type sequence"));

        REQUIRE_THAT(
            src, HasMessageComment(_A("tmain()"), "Just add 2 numbers"));

        REQUIRE_THAT(
            src, HasMessageComment(_A("tmain()"), "And now add another 2"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        const auto &A = get_participant(j, "A");

        CHECK(A.has_value());

        CHECK(A.value()["type"] == "class");
        CHECK(A.value()["name"] == "A");
        CHECK(A.value()["display_name"] == "A");
        CHECK(A.value()["namespace"] == "clanguml::t20001");
        CHECK(A.value()["source_location"]["file"] == "t20001.cc");
        CHECK(A.value()["source_location"]["line"] == 13);

        const auto &tmain = get_participant(j, "tmain()");

        CHECK(tmain.has_value());

        CHECK(tmain.value()["type"] == "function");
        CHECK(tmain.value()["name"] == "tmain");
        CHECK(tmain.value()["display_name"] == "tmain()");
        CHECK(tmain.value()["namespace"] == "clanguml::t20001");
        CHECK(tmain.value()["source_location"]["file"] == "t20001.cc");
        CHECK(tmain.value()["source_location"]["line"] == 61);

        REQUIRE(HasTitle(j, "Basic sequence diagram example"));

        REQUIRE(IsFunctionParticipant(j, "tmain()"));
        REQUIRE(IsClassParticipant(j, "A"));
        REQUIRE(IsClassParticipant(j, "B"));

        std::vector<int> messages = {
            FindMessage(j, "tmain()", "A", "add(int,int)"),
            FindMessage(j, "tmain()", "B", "wrap_add3(int,int,int)"),
            FindMessage(j, "B", "A", "add3(int,int,int)"),
            FindMessage(j, "A", "A", "add(int,int)"),
            FindMessage(j, "A", "A", "log_result(int)"),
            FindMessage(j, "B", "A", "log_result(int)")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);
        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;
        using mermaid::HasComment;
        using mermaid::HasTitle;

        REQUIRE_THAT(src, HasTitle("Basic sequence diagram example"));

        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "add3(int,int,int)"));
        REQUIRE_THAT(src, HasCall(_A("A"), "add(int,int)"));
        REQUIRE_THAT(src, !HasCall(_A("A"), _A("detail::C"), "add(int,int)"));
        REQUIRE_THAT(src, HasCall(_A("A"), "log_result(int)"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "log_result(int)"));

        REQUIRE_THAT(src, HasComment("t20001 test diagram of type sequence"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
     */
}
