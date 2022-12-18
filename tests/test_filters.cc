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

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    CHECK(cfg.diagrams.size() == 1);
    auto &config = *cfg.diagrams["include_test"];
    clanguml::include_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    auto make_path = [&](std::string_view p) {
        return source_file{config.relative_to() / p};
    };

    CHECK(filter.should_include(
        make_path("class_diagram/visitor/translation_unit_visitor.h")));
    CHECK(filter.should_include(make_path("main.cc")));
    CHECK(filter.should_include(make_path("util/util.cc")));
    CHECK_FALSE(filter.should_include(make_path("util/error.h")));
    CHECK_FALSE(filter.should_include(
        make_path("sequence_diagram/visitor/translation_unit_visitor.h")));
}
