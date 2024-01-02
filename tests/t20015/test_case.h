/**
 * tests/t20015/test_case.h
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

TEST_CASE("t20015", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20015");

    auto diagram = config.diagrams["t20015_sequence"];

    REQUIRE(diagram->name == "t20015_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20015_sequence");
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("B"),
                "setup_a(std::shared_ptr<detail::A> &)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("detail::A"), "set_x(int)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("detail::A"), "set_y(int)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("detail::A"), "set_z(int)"));

        REQUIRE_THAT(src, !HasCall(_A("B"), _A("B"), "set_x(int)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("B"), "set_y(int)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("B"), "set_z(int)"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        std::vector<int> messages = {FindMessage(
            j, "tmain()", "B", "setup_a(std::shared_ptr<detail::A> &)")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("B"),
                "setup_a(std::shared_ptr<detail::A> &)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("detail::A"), "set_x(int)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("detail::A"), "set_y(int)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("detail::A"), "set_z(int)"));

        REQUIRE_THAT(src, !HasCall(_A("B"), _A("B"), "set_x(int)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("B"), "set_y(int)"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("B"), "set_z(int)"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}