/**
 * tests/t20046/test_case.h
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

TEST_CASE("t20046")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20046", "t20046_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "tmain()::(lambda t20046.cc:13:15)",
                    "operator()(auto &&) const"}, //
                {"tmain()::(lambda t20046.cc:13:15)",
                    "tmain()::(lambda t20046.cc:13:15)::(lambda "
                    "t20046.cc:14:16)",
                    "operator()(auto &&) const"}, //
                {"tmain()::(lambda t20046.cc:13:15)::(lambda "
                 "t20046.cc:14:16)",
                    "a2(int)", ""}, //

                {"tmain()",
                    "a1<(lambda at t20046.cc:19:9)>((lambda at t20046.cc:19:9) "
                    "&&)",
                    ""},
                {"a1<(lambda at t20046.cc:19:9)>((lambda at t20046.cc:19:9) "
                 "&&)",
                    "tmain()::(lambda t20046.cc:19:9)",
                    "operator()(auto &&) const"}, //
                {"tmain()::(lambda t20046.cc:19:9)",
                    "tmain()::(lambda t20046.cc:19:9)::(lambda "
                    "t20046.cc:19:34)",
                    "operator()(auto &&) const"}, //
                {"tmain()::(lambda t20046.cc:19:9)::(lambda t20046.cc:19:34)",
                    "a3(int)", ""} //
            }));
    });
}