/**
 * tests/t20034/test_case.h
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

TEST_CASE("t20034")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20034", "t20034_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageChainsOrder(src,
            {
                //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "C", "c2()"},          //
                    {"C", "B", "b2()"},          //
                    {"B", "A", "a2()"}           //
                },                               //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "D::d2()::(lambda t20034.cc:56:18)",
                        "operator()() const"},                         //
                    {"D::d2()::(lambda t20034.cc:56:18)", "A", "a2()"} //
                },                                                     //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "A", "a2()"}           //
                },                               //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "C", "c4()"},          //
                    {"C", "B", "b4()"},          //
                    {"B", "B", "b2()"},          //
                    {"B", "A", "a2()"}           //
                },                               //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "C", "c1()"},          //
                    {"C", "B", "b1()"},          //
                    {"B", "A", "a2()"}           //
                },                               //
                {
                    //
                    {Entrypoint{}, "D", "d2()"}, //
                    {"D", "C", "c3()"},          //
                    {"C", "C", "c2()"},          //
                    {"C", "B", "b2()"},          //
                    {"B", "A", "a2()"}           //
                }                                //
            }));
    });
}