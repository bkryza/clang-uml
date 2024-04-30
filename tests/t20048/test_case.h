/**
 * tests/t20048/test_case.h
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

TEST_CASE("t20048", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20048");

    auto diagram = config.diagrams["t20048_sequence"];

    REQUIRE(diagram->name == "t20048_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20048_sequence");

    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("a3(int)"), ""));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("a2(int)"), ""));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("a1(int)"), ""));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("a5(int)"), ""));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("a6(int)"), ""));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("a7(int)"), ""));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("tmain()::(lambda t20048.cc:26:11)"),
                "operator()(auto &&) const"));
        REQUIRE_THAT(src,
            HasCall(
                _A("tmain()::(lambda t20048.cc:26:11)"), _A("a4(int)"), ""));

        REQUIRE_THAT(src,
            HasMessageComment(_A("tmain()"),
                "a1\\(\\) adds `1` to the result\\n"
                "of a2\\(\\)"));
        REQUIRE_THAT(src,
            HasMessageComment(_A("tmain()"),
                "This lambda calls a4\\(\\) which\\n"
                "adds `4` to it's argument"));
        REQUIRE_THAT(src,
            HasMessageComment(
                _A("tmain()"), "a6\\(\\) adds `1` to its argument"));
        REQUIRE_THAT(src,
            HasMessageComment(_A("tmain()"),
                "a5\\(\\) adds `1` to the result\\n"
                "of a6\\(\\)"));
        REQUIRE_THAT(src,
            HasMessageComment(
                _A("tmain()"), "a7\\(\\) is called via add std::async"));

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