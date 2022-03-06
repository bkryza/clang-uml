/**
 * tests/test_util.cc
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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
#define CATCH_CONFIG_MAIN

#include "util/util.h"

#include "catch.h"

TEST_CASE("Test split", "[unit-test]")
{
    using C = std::vector<std::string>;
    using namespace clanguml::util;

    const C empty{};

    CHECK(split("", " ") == C{""});
    CHECK(split("ABCD", " ") == C{"ABCD"});

    CHECK(split("std::vector::detail", "::") == C{"std", "vector", "detail"});

    CHECK(split("std::vector::detail::", "::") == C{"std", "vector", "detail"});
}
//
// TEST_CASE("Test ns_relative", "[unit-test]")
//{
//    using namespace clanguml::util;
//
//    CHECK(ns_relative({}, "std::vector") == "std::vector");
//    CHECK(ns_relative({"std"}, "std::vector") == "vector");
//    CHECK(ns_relative({"std"}, "const std::vector&") == "const vector&");
//    CHECK(ns_relative({"std", "clanguml::t0"},
//              "static const std::vector<clanguml::t0::a>&") ==
//        "static const vector<a>&");
//    CHECK(ns_relative({"clanguml::t0"}, "clanguml::t0") == "t0");
//}

TEST_CASE("Test abbreviate", "[unit-test]")
{
    using namespace clanguml::util;

    CHECK(abbreviate("", 10) == "");
    CHECK(abbreviate("abcd", 10) == "abcd");
    CHECK(abbreviate("abcd", 2) == "ab");
    CHECK(abbreviate("abcdefg", 5) == "ab...");
}

TEST_CASE("Test replace_all", "[unit-test]")
{
    using namespace clanguml::util;

    const std::string orig = R"(
                        Lorem ipsum \clanguml{note} something something...

                        \clanguml{style}

                        )";
    std::string text{orig};

    CHECK(replace_all(text, "NOTHERE", "HERE") == false);
    CHECK(replace_all(text, "\\clanguml", "@clanguml") == true);
    CHECK(replace_all(text, "something", "nothing") == true);
    CHECK(replace_all(text, "something", "nothing") == false);
    CHECK(replace_all(text, "nothing", "something") == true);
    CHECK(replace_all(text, "@clanguml", "\\clanguml") == true);

    CHECK(text == orig);
}
