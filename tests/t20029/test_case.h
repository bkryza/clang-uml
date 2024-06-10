/**
 * tests/t20029/test_case.h
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

TEST_CASE("t20029")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20029", "t20029_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "ConnectionPool", "connect()"}, //
                {"tmain()", "Encoder<Retrier<ConnectionPool>>",
                    "send(std::string &&)", InControlCondition{}}, //
                {"Encoder<Retrier<ConnectionPool>>",
                    "Encoder<Retrier<ConnectionPool>>",
                    "encode(std::string &&)"}, //
                {"Encoder<Retrier<ConnectionPool>>",
                    "encode_b64(std::string &&)", ""}, //
                {"Encoder<Retrier<ConnectionPool>>", "Retrier<ConnectionPool>",
                    "send(std::string &&)"}, //
                {"Retrier<ConnectionPool>", "ConnectionPool",
                    "send(const std::string &)", InControlCondition{}} //
            }));

        REQUIRE(!HasMessage(
            src, {"ConnectionPool", "ConnectionPool", "connect_impl()"}));

        REQUIRE(HasMessageComment(src, "tmain()",
            "Establish connection to the\\n"
            "remote server synchronously"));

        REQUIRE(HasMessageComment(src, "tmain()",
            "Repeat for each line in the\\n"
            "input stream"));

        REQUIRE(HasMessageComment(src, "Encoder<Retrier<ConnectionPool>>",
            "Encode the message using\\n"
            "Base64 encoding and pass\\n"
            "it to the next layer"));

        REQUIRE(HasMessageComment(src, "Retrier<ConnectionPool>",
            "Repeat until send() succeeds\\n"
            "or retry count is exceeded"));
    });
}