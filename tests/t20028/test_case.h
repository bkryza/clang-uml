/**
 * tests/t20028/test_case.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t20028", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20028");

    auto diagram = config.diagrams["t20028_sequence"];

    REQUIRE(diagram->name == "t20028_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20028_sequence");
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(
            src, HasCallInControlCondition(_A("tmain()"), _A("A"), "a()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "b()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "c()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "d()"));
        REQUIRE_THAT(src, !HasCall(_A("tmain()"), _A("B"), "e()"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        std::vector<int> messages = {FindMessage(j, "tmain()", "A", "a()"),
            FindMessage(j, "tmain()", "A", "b()"),
            FindMessage(j, "tmain()", "A", "c()"),
            FindMessage(j, "tmain()", "A", "d()")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;
        using mermaid::HasCallInControlCondition;

        REQUIRE_THAT(
            src, HasCallInControlCondition(_A("tmain()"), _A("A"), "a()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "b()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "c()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "d()"));
        REQUIRE_THAT(src, !HasCall(_A("tmain()"), _A("B"), "e()"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}