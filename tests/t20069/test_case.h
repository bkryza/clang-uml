/**
 * tests/t20069/test_case.h
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

TEST_CASE("t20069")
{
    using namespace clanguml::test;
    using namespace std::string_literals;
    {
        auto [config, db, diagram, model] =
            CHECK_SEQUENCE_MODEL("t20069", "t20069_sequence");

        CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
            REQUIRE(MessageOrder(src,
                {
                    //
                    {"t20069.cc", "A", "a(int)"},       //
                    {"A", "t20069.cc", "", Response{}}, //
                    {"A", "t20069.cc", "bar(int)"},     //
                    {"t20069.cc", "A", "", Response{}}, //
                    {"A", "t20069.cc", "", Response{}}, //

                    {"A", "A::a(int)::(lambda t20069.cc:19:22)",
                        "operator()(auto &) const"},
                    {"A::a(int)::(lambda t20069.cc:19:22)", "A", "",
                        Response{}},

                    {Exitpoint{}, "t20069.cc"}, //

                    {"t20069.cc", "t20069.cc", "baz<int>(int)"}, //
                    {"t20069.cc", "t20069.cc", "foo(int)"},      //
                    {"t20069.cc", "t20069.cc", "", Response{}},  //

                    {"t20069.cc", "t20069.cc", "bar(int)"},     //
                    {"t20069.cc", "t20069.cc", "", Response{}}, //

                    {"t20069.cc", "t20069.cc", "", Response{}}, //

                    {Exitpoint{}, "t20069.cc"}, //
                    {Exitpoint{}, "t20069.cc"}  //
                }));
        });
    }
    {
        auto [config, db, diagram, model] = CHECK_SEQUENCE_MODEL(
            "t20069", "t20069_sequence_with_return_values");

        CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
            //            REQUIRE(HasTitle(src, "AAA"));
            REQUIRE(MessageOrder(src,
                {
                    //
                    {"t20069.cc", "A", "a(int)"},             //
                    {"A", "t20069.cc", "x", Response{}},      //
                    {"A", "t20069.cc", "bar(int)"},           //
                    {"t20069.cc", "A", "x / 2", Response{}},  //
                    {"A", "t20069.cc", "bar(x)", Response{}}, //

                    {"A", "A::a(int)::(lambda t20069.cc:19:22)",
                        "operator()(auto &) const"},
                    {"A::a(int)::(lambda t20069.cc:19:22)", "A", "v + 1",
                        Response{}},

                    {"A", "t20069.cc", "f(x)", Response{}}, //
                    {"A", "t20069.cc", "0", Response{}},    //

                    {Exitpoint{}, "t20069.cc"}, //

                    {"t20069.cc", "t20069.cc", "baz<int>(int)"},     //
                    {"t20069.cc", "t20069.cc", "foo(int)"},          //
                    {"t20069.cc", "t20069.cc", "x * 2", Response{}}, //

                    {"t20069.cc", "t20069.cc", "bar(int)"},          //
                    {"t20069.cc", "t20069.cc", "x / 2", Response{}}, //

                    {"t20069.cc", "t20069.cc", "foo(x) + bar(x)",
                        Response{}}, //

                    {Exitpoint{}, "t20069.cc"}, //
                    {Exitpoint{}, "t20069.cc"}  //
                }));
        });
    }
}