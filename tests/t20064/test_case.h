/**
 * tests/t20064/test_case.h
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

TEST_CASE("t20064")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20064", "t20064_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        // REQUIRE(HasTitle(src, "Basic sequence diagram example"));

        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "Arithmetic<int,float,double>", "sum() const"}, //

                {"Arithmetic<int,float,double>", "Arithmetic<int,float,double>",
                    "sumImpl(const Arithmetic<int,float,double> &)",
                    Static{}}, //
                {"Arithmetic<int,float,double>", "HList<int,float,double>",
                    "size()", Static{}}, //
                {"Arithmetic<int,float,double>", "HList<int,float,double>",
                    "head() const"}, //
                {"Arithmetic<int,float,double>", "HList<int,float,double>",
                    "tail() const"}, //

                {"Arithmetic<int,float,double>", "Arithmetic<int,float,double>",
                    "sumImpl(const HList<float,double> &)", Static{}}, //
                {"Arithmetic<int,float,double>", "HList<float,double>",
                    "size()", Static{}}, //
                {"Arithmetic<int,float,double>", "HList<float,double>",
                    "head() const"}, //
                {"Arithmetic<int,float,double>", "HList<float,double>",
                    "tail() const"}, //

                {"Arithmetic<int,float,double>", "Arithmetic<int,float,double>",
                    "sumImpl(const HList<double> &)", Static{}}, //
                {"Arithmetic<int,float,double>", "HList<double>", "size()",
                    Static{}}, //
                {"Arithmetic<int,float,double>", "HList<double>",
                    "head() const"}, //
                {"Arithmetic<int,float,double>", "HList<double>",
                    "tail() const"}, //

                {"Arithmetic<int,float,double>", "Arithmetic<int,float,double>",
                    "sumImpl(const HList<> &)", Static{}}, //
                {"Arithmetic<int,float,double>", "HList<>", "size()",
                    Static{}} //
            }));
    });
}