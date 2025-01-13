/**
 * tests/t20066/test_case.h
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

TEST_CASE("t20066")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20066", "t20066_sequence");

    CHECK_SEQUENCE_DIAGRAM(
        *config, diagram, *model,
        [](const auto &src) {
            REQUIRE(MessageChainsOrder(src,
                {
                    //
                    {
                        {Entrypoint{}, "t20066.cc", "aaa()"}, //
                        {"t20066.cc", "t20066.cc", "aa()"},   //
                        {"t20066.cc", "t20066.cc", "a()"}     //
                    },
                    {
                        {Entrypoint{}, "t20066.cc", "baa()"}, //
                        {"t20066.cc", "t20066.cc", "ba()"},   //
                        {"t20066.cc", "t20066.cc", "a()"}     //
                    },
                    {
                        {Entrypoint{}, "t20066.cc", "bb()"}, //
                        {"t20066.cc", "t20066.cc", "b()"}    //
                    } //
                }));
            if constexpr (!std::is_same_v<json_t,
                              std::decay_t<decltype(src)>>) {
                REQUIRE(!HasMessage(src, {"t20066.cc", "t20066.cc", "cc()"}));
                REQUIRE(!HasMessage(src, {"t20066.cc", "t20066.cc", "c()"}));
            }
        },
        [](const json_t &src) { REQUIRE_EQ(src.src["sequences"].size(), 2); });
}