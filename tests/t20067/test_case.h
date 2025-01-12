/**
 * tests/t20067/test_case.h
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

TEST_CASE("t20067")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20067", "t20067_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageChainsOrder(src,
            {//
                {
                    //
                    {Entrypoint{}, "t20067.cc", "foo_bar()"}, //
                    {"t20067.cc", "t20067.cc", "bar()"},      //
                    {"t20067.cc", "t20067.cc", "baz()"}       //
                },
                {
                    //
                    {Entrypoint{}, "t20067.cc", "foo_baz()"}, //
                    {"t20067.cc", "t20067.cc", "baz()"}       //
                }}));
    });
}