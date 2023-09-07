/**
 * tests/t20034/test_case.h
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

TEST_CASE("t20034", "[test-case][sequence]")
{
    auto [config, db] = load_config("t20034");

    auto diagram = config.diagrams["t20034_sequence"];

    REQUIRE(diagram->name == "t20034_sequence");

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == "t20034_sequence");

    {
        auto puml = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(puml, HasCall(_A("D"), _A("A"), "a2()"));

        REQUIRE_THAT(puml, HasCall(_A("D"), _A("C"), "c3()"));
        REQUIRE_THAT(puml, HasCall(_A("C"), _A("C"), "c2()"));
        REQUIRE_THAT(puml, HasCall(_A("C"), _A("B"), "b2()"));
        REQUIRE_THAT(puml, HasCall(_A("B"), _A("A"), "a2()"));

        REQUIRE_THAT(puml, HasCall(_A("D"), _A("C"), "c4()"));

        REQUIRE_THAT(puml, !HasCall(_A("C"), _A("B"), "b3()"));

        save_puml(config.output_directory(), diagram->name + ".puml", puml);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        REQUIRE(HasMessageChain(j,
            {{"D::d2()", "C::c3()", "void"}, {"C::c3()", "C::c2()", "void"},
                {"C::c2()", "B::b2()", "void"},
                {"B::b2()", "A::a2()", "void"}}));
        REQUIRE(HasMessageChain(j,
            {{"D::d2()", "C::c4()", "void"}, {"C::c4()", "B::b4()", "void"},
                {"B::b4()", "B::b2()", "void"},
                {"B::b2()", "A::a2()", "void"}}));
        REQUIRE(HasMessageChain(j, {{"D::d2()", "A::a2()", "void"}}));
        REQUIRE(HasMessageChain(j,
            {{"D::d2()", "C::c1()", "void"}, {"C::c1()", "B::b1()", "void"},
                {"B::b1()", "A::a2()", "void"}}));
        REQUIRE(HasMessageChain(j,
            {{"D::d2()", "C::c2()", "void"}, {"C::c2()", "B::b2()", "void"},
                {"B::b2()", "A::a2()", "void"}}));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto mmd = generate_sequence_mermaid(diagram, *model);

        save_mermaid(config.output_directory(), diagram->name + ".mmd", mmd);
    }
}