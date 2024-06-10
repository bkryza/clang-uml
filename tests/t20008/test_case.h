/**
 * tests/t20008/test_case.h
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

TEST_CASE("t20008")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20008", "t20008_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "B<int>", "b(int)"}, //
                {"B<int>", "A<int>", "a1(int)"}, //

                {"tmain()", "B<const char *>", "b(const char *)"},          //
                {"B<const char *>", "A<const char *>", "a2(const char *)"}, //

                {"tmain()", "B<std::string>", "b(std::string)"},        //
                {"B<std::string>", "A<std::string>", "a3(std::string)"} //
            }));

        REQUIRE(!HasMessage(src, {"B<int>", "A<int>", "a2(int)"}));
        REQUIRE(!HasMessage(src, {"B<int>", "A<int>", "a3(int)"}));
    });
}