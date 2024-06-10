/**
 * tests/t20004/test_case.h
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

TEST_CASE("t20004")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20004", "t20004_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"main()", "m1<float>(float)", ""},                 //
                {"main()", "m1<unsigned long>(unsigned long)", ""}, //
                {"m1<unsigned long>(unsigned long)",
                    "m4<unsigned long>(unsigned long)", ""},    //
                {"main()", "m1<std::string>(std::string)", ""}, //
                {"m1<std::string>(std::string)", "m2<std::string>(std::string)",
                    ""},                              //
                {"main()", "m1<int>(int)", ""},       //
                {"m1<int>(int)", "m2<int>(int)", ""}, //
                {"m2<int>(int)", "m3<int>(int)", ""}, //
                {"m3<int>(int)", "m4<int>(int)", ""}  //
            }));

        REQUIRE(!HasMessage(src, {"m1<float>(float)", "m1<float>(float)", ""}));
    });
}