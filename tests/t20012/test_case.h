/**
 * test_case.h
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

TEST_CASE("t20012")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20012", "t20012_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                {"tmain()", "tmain()::(lambda t20012.cc:67:20)",
                    "operator()() const"},                         //
                {"tmain()::(lambda t20012.cc:67:20)", "A", "a()"}, //
                {"A", "A", "aa()"},                                //
                {"A", "A", "aaa()"},                               //

                {"tmain()::(lambda t20012.cc:67:20)", "B", "b()"}, //
                {"B", "B", "bb()"},                                //
                {"B", "B", "bbb()"},                               //

                {"tmain()", "tmain()::(lambda t20012.cc:80:20)",
                    "operator()() const"},                         //
                {"tmain()::(lambda t20012.cc:80:20)", "C", "c()"}, //
                {"C", "C", "cc()"},                                //
                {"C", "C", "ccc()"},                               //

                {"tmain()::(lambda t20012.cc:80:20)",
                    "tmain()::(lambda t20012.cc:67:20)",
                    "operator()() const"}, //

                {"tmain()::(lambda t20012.cc:67:20)", "A", "a()"}, //

                {"A", "A", "aa()"},  //
                {"A", "A", "aaa()"}, //

                {"tmain()::(lambda t20012.cc:67:20)", "B", "b()"}, //
                {"B", "B", "bb()"},                                //
                {"B", "B", "bbb()"},                               //

                {"tmain()", "R<(lambda at t20012.cc:86:9)>",
                    "R((lambda at t20012.cc:86:9) &&)"},             //
                {"tmain()", "R<(lambda at t20012.cc:86:9)>", "r()"}, //
                {"R<(lambda at t20012.cc:86:9)>",
                    "tmain()::(lambda t20012.cc:86:9)",
                    "operator()() const"},                        //
                {"tmain()::(lambda t20012.cc:86:9)", "C", "c()"}, //

                {"tmain()", "tmain()::(lambda t20012.cc:94:9)",
                    "operator()(auto) const"},                               //
                {"tmain()::(lambda t20012.cc:94:9)", "D", "add5(int) const"} //
            }));
    });
}
