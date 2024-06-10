/**
 * tests/t20031/test_case.h
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

TEST_CASE("t20031")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20031", "t20031_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain(int)", "magic()", ""},                             //
                {"tmain(bool,int)", "execute(std::function<int ()>)", ""}, //
                {"tmain(bool,int)", "A", "value() const"}                  //
            }));
        REQUIRE(!HasMessage(src, {"A", "A", "create()"}));
        REQUIRE(!HasMessage(src, {"tmain(int)", "A", "operator+=(int)"}));
        REQUIRE(!HasMessage(src, {"tmain(bool,int)", "A", "A()"}));
        REQUIRE(!HasMessage(src, {"tmain(bool,int)", "A", "operator+=(int)"}));
        REQUIRE(!HasMessage(src, {"A", "A", "add(int)"}));
        REQUIRE(
            !HasMessage(src, {"tmain(bool,int)", "A", "operator=(const A &)"}));
        REQUIRE(!HasMessage(src, {"A", "A", "set(int)"}));
        REQUIRE(!HasMessage(src,
            {"tmain(bool,int)::(lambda ../../tests/t20031/t20031.cc:47:26)",
                "zero()", ""}));
    });
}