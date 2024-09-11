/**
 * tests/t20056/test_case.h
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

TEST_CASE("t20056")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20056", "t20056_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                // clang-format off
                {"tmain()", "C", "c()"},
                {"C", "C", "cc()"},
                {"C", "C", "ccc()"},
                {"C", "B", "b()"},
                {"B", "B", "bb()"},
                {"B", "B", "bbb()"},
                {"B", "A", "a()"},
                {"A", "A", "aa()"},
                {"A", "A", "aaa()"},
                {"tmain()", "C", "c()"},
                {"tmain()", "C", "c()"},
                {"tmain()", "B", "b()"},
                {"tmain()", "A", "a()"},
                // clang-format on
            }));
    });
}