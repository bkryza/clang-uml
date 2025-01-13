/**
 * tests/t20068/test_case.h
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

TEST_CASE("t20068")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20068", "t20068_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {
                    //
                    {Entrypoint{}, "t20068.cc", "tmain()"}, //
                    {"t20068.cc", "t20068.cc", "cc()"},     //
                    {"t20068.cc", "t20068.cc", "c()"}       //
                } //
            }));
        REQUIRE(MessageChainsOrder(src,
            {
                //
                {
                    //
                    {Entrypoint{}, "t20068.cc", "foo()"}, //
                    {"t20068.cc", "t20068.cc", "bar()"},  //
                    {"t20068.cc", "t20068.cc", "baz()"}   //
                } //
            }));
        REQUIRE(MessageChainsOrder(src,
            {
                {
                    //
                    {Entrypoint{}, "t20068.cc", "aaa()"}, //
                    {"t20068.cc", "t20068.cc", "ab()"},   //
                    {"t20068.cc", "t20068.cc", "a()"}     //
                },                                        //
                //
                {
                    //
                    {Entrypoint{}, "t20068.cc", "aaa()"}, //
                    {"t20068.cc", "t20068.cc", "aa()"},   //
                    {"t20068.cc", "t20068.cc", "a()"}     //
                } //
            }));
    });
}