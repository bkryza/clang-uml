/**
 * tests/t20017/test_case.h
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

TEST_CASE("t20017")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20017", "t20017_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {Entrypoint{}, "t20017.cc", "tmain()"},                  //
                {"t20017.cc", "include/t20017_a.h", "a3(int,int)"},      //
                {"t20017.cc", "include/t20017_b.h", "b1(int,int)"},      //
                {"t20017.cc", "include/t20017_a.h", "a2(int,int)"},      //
                {"t20017.cc", "include/t20017_a.h", "a1(int,int)"},      //
                {"t20017.cc", "include/t20017_b.h", "b2<int>(int,int)"}, //
                {Exitpoint{}, "t20017.cc"},                              //
            }));
    });
}