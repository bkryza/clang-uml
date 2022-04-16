/**
 * tests/test_filters.cc
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

#include "catch.h"

#include "common/model/diagram_filter.h"
#include "common/model/source_file.h"
#include "config/config.h"

#include "include_diagram/model/diagram.h"

#include <filesystem>

TEST_CASE("Test diagram paths filter", "[unit-test]")
{
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::source_file;

    auto make_path = [](std::string_view p) {
        return source_file{
            std::filesystem::current_path() / "test_config_data" / p};
    };

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    CHECK(cfg.diagrams.size() == 1);
    auto &config = *cfg.diagrams["include_test"];
    clanguml::include_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    CHECK(filter.should_include(make_path("dir1/file1.h")));
    CHECK(filter.should_include(make_path("dir1/dir2/file1.h")));
    CHECK(filter.should_include(make_path("dir1/dir2/dir3/dir4/file1.h")));
    CHECK_FALSE(filter.should_include(make_path("dir1/file9.h")));
    CHECK_FALSE(filter.should_include(make_path("dir1/dir3/file1.h")));
    CHECK_FALSE(filter.should_include(make_path("dir2/dir1/file9.h")));
}
