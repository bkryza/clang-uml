/**
 * tests/t20006/test_case.h
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

TEST_CASE("t20006")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20006", "t20006_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "B<int>", "b(int)"},                            //
                {"B<int>", "A<int>", "a1(int)"},                            //
                {"tmain()", "B<std::string>", "b(std::string)"},            //
                {"B<std::string>", "A<std::string>", "a2(std::string)"},    //
                {"tmain()", "BB<int,int>", "bb1(int,int)"},                 //
                {"BB<int,int>", "AA<int>", "aa1(int)"},                     //
                {"tmain()", "BB<int,int>", "bb2(int,int)"},                 //
                {"BB<int,int>", "AA<int>", "aa2(int)"},                     //
                {"tmain()", "BB<int,std::string>", "bb1(int,std::string)"}, //
                {"BB<int,std::string>", "AA<int>", "aa2(int)"},             //
                {"tmain()", "BB<int,std::string>", "bb2(int,std::string)"}, //
                {"BB<int,std::string>", "AA<int>", "aa1(int)"},             //
                {"tmain()", "BB<int,float>", "bb1(int,float)"},             //
                {"BB<int,float>", "BB<int,float>", "bb2(int,float)"},       //
                {"BB<int,float>", "AA<int>", "aa2(int)"}                    //

            }));
    });
}