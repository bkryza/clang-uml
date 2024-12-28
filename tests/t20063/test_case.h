/**
 * tests/t20063/test_case.h
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

TEST_CASE("t20063")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20063", "t20063_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsFunctionParticipant(src, "::thirdparty::lib1::a_impl()"));
        REQUIRE(IsFunctionParticipant(src, "thirdparty::lib1::a_impl()"));
        REQUIRE(IsClassParticipant(src, "::t00089_A"));
        REQUIRE(IsClassParticipant(src, "::thirdparty::lib1::A"));
        REQUIRE(IsClassParticipant(src, "thirdparty::lib1::A"));
        REQUIRE(IsClassParticipant(src, "thirdparty::lib3::C"));

        REQUIRE(MessageOrder(src,
            {                                                //
                {"tmain()", "::t00089_A", "A()"},            //
                {"tmain()", "::thirdparty::lib1::A", "a()"}, //
                {"tmain()", "thirdparty::lib1::A", "a()"},   //
                {"tmain()", "thirdparty::lib3::C", "c()"}}   //
            ));
    });
}