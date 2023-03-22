/**
 * tests/t20018/test_case.h
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

TEST_CASE("t20018", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20018");

    auto diagram = config.diagrams["t20018_sequence"];

    REQUIRE(diagram->name == "t20018_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20018_sequence");

    {
        auto puml = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(puml,
            HasCall(
                _A("tmain()"), _A("Answer<Factorial<5>,120>"), "__print()__"));
        REQUIRE_THAT(puml,
            HasCall(_A("Answer<Factorial<5>,120>"), _A("Factorial<5>"),
                "__print(int)__"));
        REQUIRE_THAT(puml,
            HasCall(_A("Factorial<5>"), _A("Factorial<4>"), "__print(int)__"));
        REQUIRE_THAT(puml,
            HasCall(_A("Factorial<4>"), _A("Factorial<3>"), "__print(int)__"));
        REQUIRE_THAT(puml,
            HasCall(_A("Factorial<3>"), _A("Factorial<2>"), "__print(int)__"));
        REQUIRE_THAT(puml,
            HasCall(_A("Factorial<2>"), _A("Factorial<1>"), "__print(int)__"));
        REQUIRE_THAT(puml,
            HasCall(_A("Factorial<1>"), _A("Factorial<0>"), "__print(int)__"));

        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }
}