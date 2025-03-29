/**
 * @file tests/test_levenshtein.cc
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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "util/levenshtein.h"

#include "doctest/doctest.h"

#include <string>
#include <vector>

TEST_CASE("get_approximate_matches returns empty vector for empty input")
{
    using namespace clanguml::util;
    std::vector<std::string> items;
    auto result = get_approximate_matches(items, "pattern");
    CHECK(result.empty());
}

TEST_CASE(
    "get_approximate_matches returns single element when one item is provided")
{
    using namespace clanguml::util;
    std::vector<std::string> items = {"hello"};
    auto result = get_approximate_matches(items, "hello");
    CHECK(result.size() == 1);
    CHECK(result[0] == "hello");
}

TEST_CASE("get_approximate_matches returns all items when less than three are "
          "provided")
{
    using namespace clanguml::util;
    std::vector<std::string> items = {"hello", "world"};
    auto result = get_approximate_matches(items, "hello");
    CHECK(result.size() == 2);
    CHECK(result.front() == "hello");
}

TEST_CASE(
    "get_approximate_matches selects the best three matches (ties allowed)")
{
    using namespace clanguml::util;
    std::vector<std::string> items = {"abcd", "abc", "abxd", "efgh"};
    auto result = get_approximate_matches(items, "abcd");

    CHECK(result.size() == 3);
    CHECK(result[0] == "abcd");
    CHECK((result[1] == "abc" || result[1] == "abxd"));
    CHECK((result[2] == "abc" || result[2] == "abxd"));
}

TEST_CASE(
    "get_approximate_matches returns top three from more than three candidates")
{
    using namespace clanguml::util;

    std::vector<std::string> items = {
        "apple", "aple", "apply", "bpple", "orange"};
    auto result = get_approximate_matches(items, "apple");

    CHECK(result.size() == 3);
    CHECK(result[0] == "apple");

    bool second_is_valid =
        (result[1] == "aple" || result[1] == "apply" || result[1] == "bpple");
    bool third_is_valid =
        (result[2] == "aple" || result[2] == "apply" || result[2] == "bpple");

    CHECK(second_is_valid);
    CHECK(third_is_valid);
}