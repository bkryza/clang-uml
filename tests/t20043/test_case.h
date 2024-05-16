/**
 * tests/t20043/test_case.h
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

TEST_CASE("t20043")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20043", "t20043_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "D", "d()"}, //
                {"D", "C", "c()"},       //
            }));

        REQUIRE(!HasMessage(src, {"tmain()", "F", "f()"}));

        REQUIRE(!HasMessage(src, {"D", {"detail", "E"}, "e()"}));
        REQUIRE(!HasMessage(src, {"C", "B", "b()"}));
        REQUIRE(!HasMessage(src, {"B", "A", "a()"}));
    });
/*
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, !HasCall(_A("tmain()"), _A("F"), "f()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("D"), "d()"));

        REQUIRE_THAT(src, HasCall(_A("D"), _A("C"), "c()"));
        REQUIRE_THAT(src, !HasCall(_A("D"), _A("detail::E"), "e()"));
        REQUIRE_THAT(src, !HasCall(_A("C"), _A("B"), "b()"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("A"), "a()"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;

        REQUIRE_THAT(src, !HasCall(_A("tmain()"), _A("F"), "f()"));
        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("D"), "d()"));

        REQUIRE_THAT(src, HasCall(_A("D"), _A("C"), "c()"));
        REQUIRE_THAT(src, !HasCall(_A("D"), _A("detail::E"), "e()"));
        REQUIRE_THAT(src, !HasCall(_A("C"), _A("B"), "b()"));
        REQUIRE_THAT(src, !HasCall(_A("B"), _A("A"), "a()"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }*/
}