/**
 * tests/t20044/test_case.h
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

TEST_CASE("t20044", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20044");

    auto diagram = config.diagrams["t20044_sequence"];

    REQUIRE(diagram->name == "t20044_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20044_sequence");

    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src,
            HasCall(
                _A("tmain()"), _A("R"), "R((lambda at t20044.cc:74:9) &&)"));
        REQUIRE_THAT(src,
            HasCall(_A("R"), _A("tmain()::(lambda t20044.cc:74:9)"),
                "operator()()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20044.cc:74:9)"), _A("A"), "a()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("tmain()::(lambda t20044.cc:84:18)"),
                "operator()()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20044.cc:84:18)"), _A("A"), "a5()"));

        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "a1()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("detail::expected<int,error>"),
                "and_then((lambda at t20044.cc:90:19) &&)"));

        REQUIRE_THAT(src,
            HasCall(_A("detail::expected<int,error>"),
                _A("tmain()::(lambda t20044.cc:90:19)"), "operator()()"));

        REQUIRE_THAT(src,
            HasCall(
                _A("tmain()::(lambda t20044.cc:90:19)"), _A("A"), "a2(int)"));

        REQUIRE_THAT(src,
            HasCall(
                _A("A"), _A("detail::expected<int,error>"), "expected(int)"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsClass;

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}