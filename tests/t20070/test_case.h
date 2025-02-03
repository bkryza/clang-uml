/**
 * tests/t20070/test_case.h
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

TEST_CASE("t20070")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20070", "t20070_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"fibonacci_sequence(unsigned int)", "foo()", ""}, //
                {"fibonacci_sequence(unsigned int)", "tmain()",
                    "Generator<unsigned long long>", Response{}, CoReturn{}}, //
                {"fibonacci_sequence(unsigned int)", "AwaitableFoo",
                    "await_resume() const", CoAwait()}, //

                {"fibonacci_sequence(unsigned int)", "Generator::promise_type",
                    "yield_value(int &&)"}, //
                {"Generator::promise_type", "fibonacci_sequence(unsigned int)",
                    "std::suspend_always", Response{}},
                {"fibonacci_sequence(unsigned int)", "tmain()",
                    "Generator<unsigned long long>", Response{}, CoYield{}}, //
                {"fibonacci_sequence(unsigned int)", "tmain()",
                    "Generator<unsigned long long>", Response{}, CoReturn{}}, //

                {"fibonacci_sequence(unsigned int)", "Generator::promise_type",
                    "yield_value(unsigned long long &)"}, //
                {"Generator::promise_type", "fibonacci_sequence(unsigned int)",
                    "std::suspend_always", Response{}},
                {"fibonacci_sequence(unsigned int)", "tmain()",
                    "Generator<unsigned long long>", Response{}, CoYield{}}, //

                {"tmain()", "Generator<unsigned long long>", "operator bool()",
                    InControlCondition{}}, //
                {"Generator<unsigned long long>",
                    "Generator<unsigned long long>", "fill()"},

                {"tmain()", "Generator<unsigned long long>", "operator()()"}, //
                {"Generator<unsigned long long>",
                    "Generator<unsigned long long>", "fill()"}, //
                {"Generator<unsigned long long>", "tmain()",
                    "unsigned long long", Response{}} //

            }));
    });
}
