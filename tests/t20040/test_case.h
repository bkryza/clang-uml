/**
 * tests/t20040/test_case.h
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

TEST_CASE("t20040")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20040", "t20040_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()",
                    "print<int,double,std::string>(int,double,std::string)",
                    ""}, //
                {"print<int,double,std::string>(int,double,std::string)",
                    "print<double,std::string>(double,std::string)", ""}, //
                {"print<double,std::string>(double,std::string)",
                    "print<std::string>(std::string)", ""},         //
                {"print<std::string>(std::string)", "print()", ""}, //

                {"tmain()", "doublePrint<std::string,int>(std::string,int)",
                    ""}, //
                {"doublePrint<std::string,int>(std::string,int)",
                    "print<std::string,int>(std::string,int)", ""}, //
                {"print<std::string,int>(std::string,int)", "print<int>(int)",
                    ""},                            //
                {"print<int>(int)", "print()", ""}, //
            }));
    });
}