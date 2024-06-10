/**
 * tests/t20044/test_case.h
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

TEST_CASE("t20044")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20044", "t20044_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(!IsClassParticipant(src, "detail::expected<int,error>"));
        REQUIRE(IsClassParticipant(src, "result_t"));

        REQUIRE(MessageOrder(src,
            {
                {"tmain()", "R", "R((lambda at t20044.cc:74:9) &&)"}, //
                {"R", "tmain()::(lambda t20044.cc:74:9)",
                    "operator()() const"},                              //
                {"tmain()::(lambda t20044.cc:74:9)", "A", "a() const"}, //

                {"tmain()", "tmain()::(lambda t20044.cc:84:18)",
                    "operator()() const"},                          //
                {"tmain()::(lambda t20044.cc:84:18)", "A", "a5()"}, //

                {"tmain()", "A", "a1() const"},     //
                {"A", "result_t", "expected(int)"}, //

                {"tmain()", "result_t",
                    "and_then((lambda at t20044.cc:90:19) &&)"}, //
                {"result_t", "tmain()::(lambda t20044.cc:90:19)",
                    "operator()(auto &&) const"},                            //
                {"tmain()::(lambda t20044.cc:90:19)", "A", "a2(int) const"}, //
                {"A", "result_t", "expected(int)"},                          //

                {"tmain()", "result_t",
                    "and_then(result_t (&)(int))"}, //                                                            //
                {"tmain()", "result_t",
                    "and_then(std::function<result_t (int)> &)"}, //                                                            //
                {"tmain()", "result_t",
                    "value() const"}, //                                                            //
            }));
    });
}