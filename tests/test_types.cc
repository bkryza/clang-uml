/**
 * @file tests/test_types.cc
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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "common/types.h"

#include "doctest/doctest.h"

TEST_CASE("Test eid_t")
{
    using clanguml::common::eid_t;

    eid_t empty{};

    REQUIRE(empty.is_global());
    REQUIRE_EQ(empty.value(), 0);

    eid_t local_id{(int64_t)100};
    REQUIRE_EQ(local_id.ast_local_value(), 100);
    REQUIRE_EQ(local_id.value(), 100);
    REQUIRE(!local_id.is_global());

    eid_t global_id{(uint64_t)100};
    REQUIRE_EQ(global_id.value(), 100);
    REQUIRE(global_id.is_global());

    REQUIRE(local_id != global_id);

    REQUIRE(local_id != 101);
}

TEST_CASE("Test to_string")
{
    using namespace clanguml::common;

    std::string t1{"abcd"};
    REQUIRE_EQ(t1, to_string(t1));

    string_or_regex t2{"abcdef"};
    REQUIRE_EQ(to_string(t2), "abcdef");
    REQUIRE_EQ(to_string(t2), t2.to_string());

    string_or_regex t3{std::regex{"ab.*"}, "ab.*"};
    REQUIRE_EQ(to_string(t3), "ab.*");
    REQUIRE_EQ(to_string(t3), t3.to_string());

    REQUIRE_EQ(to_string(generator_type_t::plantuml), "plantuml");
    REQUIRE_EQ(to_string(generator_type_t::mermaid), "mermaid");
    REQUIRE_EQ(to_string(generator_type_t::json), "json");
}