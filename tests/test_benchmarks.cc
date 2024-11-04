/**
 * @file tests/test_benchmarks.cc
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

#include "util/util.h"

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench/nanobench.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("nanobench clanguml::util::is_relative_to")
{
    using std::filesystem::path;
    using namespace clanguml::util;
    path child{"/a/b/c/d/include.h"};
    path base1{"/a/b/c"};
    path base2{"/a/b/d"};

    ankerl::nanobench::Bench().run(
        "is_relative_to positive", [&] { is_relative_to(child, base1); });

    ankerl::nanobench::Bench().run(
        "is_relative_to negative", [&] { is_relative_to(child, base2); });
}