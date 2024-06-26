/**
 * @file tests/test_filters_advanced.cc
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
#define DOCTEST_CONFIG_IMPLEMENT

#include "doctest/doctest.h"

#include "class_diagram/model/class.h"
#include "cli/cli_handler.h"
#include "common/model/filters/diagram_filter_factory.h"
#include "common/model/source_file.h"
#include "config/config.h"
#include "include_diagram/model/diagram.h"
#include "sequence_diagram/model/diagram.h"

#include <filesystem>
using clanguml::common::model::diagram_filter;
using clanguml::common::model::diagram_filter_factory;
using clanguml::common::model::namespace_;
using clanguml::common::model::source_file;
using clanguml::config::filter_mode_t;

TEST_CASE("Test advanced diagram filter anyof")
{
    auto cfg =
        clanguml::config::load("./test_config_data/filters_advanced.yml");

    auto &config = *cfg.diagrams["anyof_test"];
    clanguml::include_diagram::model::diagram diagram;

    auto filter_ptr = diagram_filter_factory::create(diagram, config);
    diagram_filter &filter = *filter_ptr;

    CHECK(config.filter_mode() == filter_mode_t::advanced);
    CHECK(filter.should_include(namespace_{"ns1::ns2"}));
    CHECK_FALSE(filter.should_include(namespace_{"std::string"}));

    clanguml::common::model::element std_thread{{}};
    std_thread.set_namespace(namespace_{"std"});
    std_thread.set_name("thread");
    CHECK(filter.should_include(std_thread));

    std_thread.set_name("jthread");
    CHECK_FALSE(filter.should_include(std_thread));

    CHECK_FALSE(filter.should_include(namespace_{"ns1::ns2::detail"}));
}

TEST_CASE("Test advanced diagram filter modules")
{
    auto cfg =
        clanguml::config::load("./test_config_data/filters_advanced.yml");

    auto &config = *cfg.diagrams["modules_test"];
    clanguml::include_diagram::model::diagram diagram;

    auto filter_ptr = diagram_filter_factory::create(diagram, config);
    diagram_filter &filter = *filter_ptr;

    CHECK(config.filter_mode() == filter_mode_t::advanced);
    CHECK(filter.should_include(namespace_{"ns1::ns2"}));
    CHECK_FALSE(filter.should_include(namespace_{"std::string"}));

    clanguml::common::model::element std_string{{}};
    std_string.set_namespace(namespace_{"std"});
    std_string.set_name("string");

    CHECK_FALSE(filter.should_include(std_string));

    CHECK(filter.should_include(namespace_{"ns1"}));

    clanguml::common::model::element e1{{}};
    e1.set_module("mod1::mod2");
    e1.set_namespace(namespace_{"ns5::ns6"});
    e1.set_name("ClassA");
    CHECK(filter.should_include(e1));

    e1.set_module("mod1::mod3");
    CHECK_FALSE(filter.should_include(e1));
}

///
/// Main test function
///
int main(int argc, char *argv[])
{
    doctest::Context context;

    context.applyCommandLine(argc, argv);

    clanguml::cli::cli_handler clih;

    std::vector<const char *> argvv = {
        "clang-uml", "--config", "./test_config_data/simple.yml"};

    argvv.push_back("-q");

    clih.handle_options(argvv.size(), argvv.data());

    int res = context.run();

    if (context.shouldExit())
        return res;

    return res;
}
