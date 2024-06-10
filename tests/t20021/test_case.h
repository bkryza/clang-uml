/**
 * tests/t20021/test_case.h
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

TEST_CASE("t20021")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20021", "t20021_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "C", "c4()", InControlCondition{}},       //
                {"C", "C", "c5()"},                                   //
                {"tmain()", "A", "a3()"},                             //
                {"tmain()", "A", "a2()", InControlCondition{}},       //
                {"tmain()", "C", "c1()", InControlCondition{}},       //
                {"tmain()", "C", "c2()", InControlCondition{}},       //
                {"tmain()", "A", "a1()"},                             //
                {"tmain()", "C", "c3()", InControlCondition{}},       //
                {"tmain()", "B", "b2() const"},                       //
                {"tmain()", "C", "contents()", InControlCondition{}}, //
                {"tmain()", "B", "b2() const"},                       //
            }));
    });
}