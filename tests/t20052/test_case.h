/**
 * tests/t20052/test_case.h
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

TEST_CASE("t20052")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20052", "t20052_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(!IsFileParticipant(src, "t20052.cu"));

        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "A", "a()"}, //
                {"A", "A", "aa()"},      //
                {"A", "A", "aaa()"},     //

                {"tmain()", "B", "b()"}, //
                {"B", "B", "bb()"},      //
                {"B", "B", "bbb()"},     //

                {"tmain()", "C", "c()"}, //
                {"C", "C", "cc()"},      //
                {"C", "C", "ccc()"},     //

                {"tmain()", "A", "a()"}, //
                {"A", "A", "aa()"},      //
                {"A", "A", "aaa()"},     //

                {"tmain()", "B", "b()"}, //
                {"B", "B", "bb()"},      //
                {"B", "B", "bbb()"},     //

                {"tmain()", "R<(lambda at t20052.cc:86:9)>",
                    "R((lambda at t20052.cc:86:9) &&)"}, //

                {"tmain()", "R<(lambda at t20052.cc:86:9)>", "r()"}, //
                {"R<(lambda at t20052.cc:86:9)>", "C", "c()"},       //
                {"C", "C", "cc()"},                                  //
                {"C", "C", "ccc()"},                                 //

                {"tmain()", "D", "add5(int) const"} //
            }));
    });
}