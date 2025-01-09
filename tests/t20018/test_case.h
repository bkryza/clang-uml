/**
 * tests/t20018/test_case.h
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

TEST_CASE("t20018")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20018", "t20018_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "Answer<Factorial<5>,120>", "print()", Static{}}, //
                {"Answer<Factorial<5>,120>", "Factorial<5>", "print(int)",
                    Static{}},                                            //
                {"Factorial<5>", "Factorial<4>", "print(int)", Static{}}, //
                {"Factorial<4>", "Factorial<3>", "print(int)", Static{}}, //
                {"Factorial<3>", "Factorial<2>", "print(int)", Static{}}, //
                {"Factorial<2>", "Factorial<1>", "print(int)", Static{}}, //
                {"Factorial<1>", "Factorial<0>", "print(int)", Static{}}, //
            }));
    });
}