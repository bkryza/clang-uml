/**
 * tests/t20017/test_case.h
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

TEST_CASE("t20017")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20017", "t20017_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
        //        {Entrypoint{}, "t20017.cc", "tmain()"},                  //
                 {"t20017.cc", "include/t20017_a.h", "a3(int,int)"},      //
                {"t20017.cc", "include/t20017_b.h", "b1(int,int)"},      //
          /*     {"t20017.cc", "include/t20017_a.h", "a2(int,int)"},      //
                {"t20017.cc", "include/t20017_a.h", "a1(int,int)"},      //
                {"t20017.cc", "include/t20017_b.h", "b2<int>(int,int)"}, //
                {Exitpoint{}, "t20017.cc"},                              //*/
            }));
    });
    /*
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src, HasEntrypoint(_A("t20017.cc"), "tmain()"));
        REQUIRE_THAT(src,
            HasCall(_A("t20017.cc"), _A("include/t20017_a.h"), "a1(int,int)"));
        REQUIRE_THAT(src,
            HasCall(_A("t20017.cc"), _A("include/t20017_a.h"), "a2(int,int)"));
        REQUIRE_THAT(src,
            HasCall(_A("t20017.cc"), _A("include/t20017_a.h"), "a3(int,int)"));
        REQUIRE_THAT(src,
            HasCall(_A("t20017.cc"), _A("include/t20017_b.h"), "b1(int,int)"));
        REQUIRE_THAT(src,
            HasCall(
                _A("t20017.cc"), _A("include/t20017_b.h"), "b2<int>(int,int)"));
        REQUIRE_THAT(src, HasExitpoint(_A("t20017.cc")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        const auto &t20017_cc = get_participant(j, "t20017.cc");

        CHECK(t20017_cc.has_value());

        CHECK(t20017_cc.value()["type"] == "file");
        CHECK(t20017_cc.value()["name"] == "t20017.cc");
        CHECK(t20017_cc.value()["display_name"] == "t20017.cc");

        std::vector<int> messages = {
            FindMessage(j, File("t20017.cc"), File("include/t20017_a.h"),
                "a3(int,int)"),
            FindMessage(j, File("t20017.cc"), File("include/t20017_b.h"),
                "b1(int,int)"),
            FindMessage(j, File("t20017.cc"), File("include/t20017_a.h"),
                "a2(int,int)"),
            FindMessage(j, File("t20017.cc"), File("include/t20017_a.h"),
                "a1(int,int)"),
            FindMessage(j, File("t20017.cc"), File("include/t20017_b.h"),
                "b2<int>(int,int)")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;
        using mermaid::HasEntrypoint;
        using mermaid::HasExitpoint;

        REQUIRE_THAT(src, HasEntrypoint(_A("t20017.cc"), "tmain()"));
        REQUIRE_THAT(src,
            HasCall(_A("t20017.cc"), _A("include/t20017_a.h"), "a1(int,int)"));
        REQUIRE_THAT(src,
            HasCall(_A("t20017.cc"), _A("include/t20017_a.h"), "a2(int,int)"));
        REQUIRE_THAT(src,
            HasCall(_A("t20017.cc"), _A("include/t20017_a.h"), "a3(int,int)"));
        REQUIRE_THAT(src,
            HasCall(_A("t20017.cc"), _A("include/t20017_b.h"), "b1(int,int)"));
        REQUIRE_THAT(src,
            HasCall(
                _A("t20017.cc"), _A("include/t20017_b.h"), "b2<int>(int,int)"));
        REQUIRE_THAT(src, HasExitpoint(_A("t20017.cc")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }*/
}