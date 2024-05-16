/**
 * tests/t20019/test_case.h
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

TEST_CASE("t20019")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20019", "t20019_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "Base<D1>", "name()"}, //
                {"Base<D1>", "D1", "impl()"},      //
                {"tmain()", "Base<D2>", "name()"}, //
                {"Base<D2>", "D2", "impl()"}      //
            }));
    });
/*
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("Base<D1>"), "name()"));
        REQUIRE_THAT(src, HasCall(_A("Base<D1>"), _A("D1"), "impl()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("Base<D2>"), "name()"));
        REQUIRE_THAT(src, HasCall(_A("Base<D2>"), _A("D2"), "impl()"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        std::vector<int> messages = {
            FindMessage(j, "tmain()", "Base<D1>", "name()"),
            FindMessage(j, "Base<D1>", "D1", "impl()"),
            FindMessage(j, "tmain()", "Base<D2>", "name()"),
            FindMessage(j, "Base<D2>", "D2", "impl()")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;

        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("Base<D1>"), "name()"));
        REQUIRE_THAT(src, HasCall(_A("Base<D1>"), _A("D1"), "impl()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("Base<D2>"), "name()"));
        REQUIRE_THAT(src, HasCall(_A("Base<D2>"), _A("D2"), "impl()"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
    */
}