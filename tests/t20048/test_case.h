/**
 * tests/t20048/test_case.h
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

TEST_CASE("t20048")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20048", "t20048_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "a3(int)", ""}, //
                {"tmain()", "a2(int)", ""}, //
                {"tmain()", "a1(int)", ""}, //
                {"tmain()", "tmain()::(lambda t20048.cc:26:11)",
                    "operator()(auto &&) const"},                     //
                {"tmain()::(lambda t20048.cc:26:11)", "a4(int)", ""}, //
                {"tmain()", "a6(int)", ""},                           //
                {"tmain()", "a5(int)", ""},                           //
                {"tmain()", "a7(int)", ""},                           //

            }));
        REQUIRE(HasMessageComment(src, "tmain()",
            "a1() adds `1` to the result\\n"
            "of a2()"));
        REQUIRE(HasMessageComment(src, "tmain()",
            "This lambda calls a4() which\\n"
            "adds `4` to it's argument"));
        REQUIRE(
            HasMessageComment(src, "tmain()", "a6() adds `1` to its argument"));
        REQUIRE(HasMessageComment(src, "tmain()",
            "a5() adds `1` to the result\\n"
            "of a6()"));
        REQUIRE(HasMessageComment(
            src, "tmain()", "a7() is called via add std::async"));
    });
}