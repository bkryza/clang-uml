/**
 * tests/t20008/test_case.h
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

TEST_CASE("t20008", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20008");

    auto diagram = config.diagrams["t20008_sequence"];

    REQUIRE(diagram->name == "t20008_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20008_sequence");

    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B<int>"), "b(int)"));
        REQUIRE_THAT(src, HasCall(_A("B<int>"), _A("A<int>"), "a1(int)"));
        //    REQUIRE_THAT(puml, !HasCall(_A("B<int>"), _A("A<int>"),
        //    "a2(int)")); REQUIRE_THAT(puml, !HasCall(_A("B<int>"),
        //    _A("A<int>"), "a3(int)"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("B<const char *>"), "b(const char *)"));
        REQUIRE_THAT(src,
            HasCall(_A("B<const char *>"), _A("A<const char *>"),
                "a2(const char *)"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("B<std::string>"), "b(std::string)"));
        REQUIRE_THAT(src,
            HasCall(
                _A("B<std::string>"), _A("A<std::string>"), "a3(std::string)"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        std::vector<int> messages = {
            FindMessage(j, "tmain()", "B<int>", "b(int)"),
            FindMessage(j, "B<int>", "A<int>", "a1(int)"),
            FindMessage(j, "tmain()", "B<const char *>", "b(const char *)"),
            FindMessage(
                j, "B<const char *>", "A<const char *>", "a2(const char *)"),
            FindMessage(j, "tmain()", "B<std::string>", "b(std::string)"),
            FindMessage(
                j, "B<std::string>", "A<std::string>", "a3(std::string)")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;

        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("B<int>"), "b(int)"));
        REQUIRE_THAT(src, HasCall(_A("B<int>"), _A("A<int>"), "a1(int)"));
        //    REQUIRE_THAT(puml, !HasCall(_A("B<int>"), _A("A<int>"),
        //    "a2(int)")); REQUIRE_THAT(puml, !HasCall(_A("B<int>"),
        //    _A("A<int>"), "a3(int)"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("B<const char *>"), "b(const char *)"));
        REQUIRE_THAT(src,
            HasCall(_A("B<const char *>"), _A("A<const char *>"),
                "a2(const char *)"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("B<std::string>"), "b(std::string)"));
        REQUIRE_THAT(src,
            HasCall(
                _A("B<std::string>"), _A("A<std::string>"), "a3(std::string)"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}