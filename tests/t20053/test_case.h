/**
 * tests/t20053/test_case.h
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t20053")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20053", "t20053_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(!IsFileParticipant(src, "t20053.cc"));

        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "a2(int)", ""},              //
                {"a2(int)", "tmain()", "2", Response{}}, //

                {"tmain()",
                    "a1<(lambda at t20053.cc:30:9)>((lambda at t20053.cc:30:9) "
                    "&&)",
                    ""}, //

                {"a1<(lambda at t20053.cc:30:9)>((lambda at t20053.cc:30:9) "
                 "&&)",
                    "a3(int)", ""}, //
                {"a3(int)",
                    "a1<(lambda at t20053.cc:30:9)>((lambda at t20053.cc:30:9) "
                    "&&)",
                    "3", Response{}}, //

                {"a1<(lambda at t20053.cc:30:9)>((lambda at t20053.cc:30:9) "
                 "&&)",
                    "tmain()", "f(42)", Response{}}, //

                {"tmain()", "a4(int)", ""},                                 //
                {"a4(int)", "tmain()", "[]() { return 5; }()", Response{}}, //
            }));
    });
}