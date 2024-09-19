/**
 * tests/t20057/test_case.h
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

TEST_CASE("t20057")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20057", "t20057_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        // REQUIRE(HasTitle(src, "Basic sequence diagram example"));

        REQUIRE(MessageOrder(src,
            {
                //
                {"t20057_tmain()", "t20057_C", "c()"}, //
                {"t20057_C", "t20057_B", "b()"},       //
                {"t20057_B", "t20057_A", "a()"}        //
            }));

        // REQUIRE(!HasMessage(src, {"A", {"detail", "C"}, "add(int,int)"}));

        // REQUIRE(HasComment(src, "t20001 test diagram of type sequence"));

        // REQUIRE(HasMessageComment(src, "tmain()", "Just add 2 numbers"));
    });
}