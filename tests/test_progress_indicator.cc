/**
 * @file tests/test_progress_indicator.cc
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

#include "doctest/doctest.h"

#include "common/generators/progress_indicator.h"
#include "util/util.h"

TEST_CASE("Test progress indicator")
{
    using namespace clanguml::common::generators;
    using namespace std::string_literals;
    using clanguml::util::contains;

    std::stringstream sstr;

    progress_indicator pi{sstr};

    pi.add_progress_bar("One", 100, indicators::Color::green);

    // Check if progress indicator has been displayed on the terminal
    pi.increment("One");

    pi.complete("One");

    pi.stop();

    std::vector<std::string> output_lines;
    std::string line;
    while (std::getline(sstr, line, '\r'))
        output_lines.emplace_back(std::move(line));

    REQUIRE_EQ(output_lines[0], "");
#ifdef _MSC_VER
    REQUIRE(contains(output_lines[1],
        "One                      [>----------------------------------] "
        "[00:00s] 0/100"s));

    REQUIRE(contains(output_lines[2],
        "One                      [>----------------------------------] "
        "[00m:00s] 1/100"s));

    REQUIRE(contains(output_lines[3],
        "One                      [===================================] "
        "[00m:00s] 100/100 OK"s));
#else
    REQUIRE_EQ(output_lines[1],
        "One                      [█----------------------------------] "
        "[00:00s] 0/100");

    REQUIRE_EQ(output_lines[2],
        "One                      [█----------------------------------] "
        "[00m:00s] 1/100");

    REQUIRE_EQ(output_lines[3],
        "One                      [███████████████████████████████████] "
        "[00m:00s] 100/100 ✔");
#endif
}

TEST_CASE("Test progress indicator fail")
{
    using namespace clanguml::common::generators;
    using namespace std::string_literals;
    using clanguml::util::contains;

    std::stringstream sstr;

    progress_indicator pi{sstr};

    pi.add_progress_bar("One", 100, indicators::Color::green);

    // Check if progress indicator has been displayed on the terminal
    pi.increment("One");

    pi.increment("Two"); // This shouldn't lock the progress bar or change it

    pi.complete("Two"); // This shouldn't lock the progress bar or change it

    pi.fail("Two"); // This shouldn't lock the progress bar or change it

    pi.fail("One");

    pi.stop();

    std::vector<std::string> output_lines;
    std::string line;
    while (std::getline(sstr, line, '\r'))
        output_lines.emplace_back(std::move(line));

    REQUIRE_EQ(output_lines[0], "");

#ifdef _MSC_VER
    REQUIRE(contains(output_lines[1],
        "One                      [>----------------------------------] "
        "[00:00s] 0/100"s));

    REQUIRE(contains(output_lines[2],
        "One                      [>----------------------------------] "
        "[00m:00s] 1/100"s));

    REQUIRE(contains(output_lines[3],
        "One                      [>----------------------------------] "
        "[00m:00s] 1/100 FAILED"s));
#else
    REQUIRE_EQ(output_lines[1],
        "One                      [█----------------------------------] "
        "[00:00s] 0/100");

    REQUIRE_EQ(output_lines[2],
        "One                      [█----------------------------------] "
        "[00m:00s] 1/100");

    REQUIRE_EQ(output_lines[3],
        "One                      [█----------------------------------] "
        "[00m:00s] 1/100 ✗");
#endif
}