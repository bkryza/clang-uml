/**
 * @file tests/test_filters_advanced.cc
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
using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_method;
using clanguml::common::eid_t;
using clanguml::common::model::access_t;
using clanguml::common::model::diagram_filter;
using clanguml::common::model::diagram_filter_factory;
using clanguml::common::model::namespace_;
using clanguml::common::model::package;
using clanguml::common::model::source_file;
using clanguml::config::filter_mode_t;

TEST_CASE("Test diagram paths filter")
{
    auto cfg =
        clanguml::config::load("./test_config_data/filters_advanced.yml");

    auto &config = *cfg.diagrams["include_test"];
    clanguml::include_diagram::model::diagram diagram;

    auto filter_ptr = diagram_filter_factory::create(diagram, config);
    diagram_filter &filter = *filter_ptr;

    auto make_path = [&](std::string_view p) {
        return source_file{config.root_directory() / p};
    };

    CHECK(filter.should_include(
        make_path("class_diagram/visitor/translation_unit_visitor.h")));
    CHECK(filter.should_include(make_path("main.cc")));
    CHECK(filter.should_include(make_path("util/util.cc")));
    CHECK_FALSE(filter.should_include(make_path("util/error.h")));
    CHECK_FALSE(filter.should_include(
        make_path("sequence_diagram/visitor/translation_unit_visitor.h")));
}

TEST_CASE("Test advanced diagram filter anyof")
{
    auto cfg =
        clanguml::config::load("./test_config_data/filters_advanced.yml");

    auto &config = *cfg.diagrams["anyof_test"];
    clanguml::include_diagram::model::diagram diagram;

    diagram.set_complete(true);

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

TEST_CASE("Test advanced diagram filter anyof with paths")
{
    auto cfg =
        clanguml::config::load("./test_config_data/filters_advanced.yml");

    auto &config = *cfg.diagrams["anyof_paths_test"];
    clanguml::include_diagram::model::diagram diagram;

    diagram.set_complete(true);

    auto filter_ptr = diagram_filter_factory::create(diagram, config);
    diagram_filter &filter = *filter_ptr;

    CHECK(config.filter_mode() == filter_mode_t::advanced);

    clanguml::common::model::element std_thread{{}};
    std_thread.set_namespace(namespace_{"std"});
    std_thread.set_name("thread");
    std_thread.set_file("/usr/include/thread");
    CHECK(filter.should_include(std_thread));

    std_thread.set_name("jthread");
    CHECK_FALSE(filter.should_include(std_thread));

    clanguml::common::model::element myclass{{}};
    myclass.set_namespace(namespace_{"myns"});
    myclass.set_name("myclass");

    auto myclass_path = config.root_directory() / "include/myclass.h";

    myclass.set_file(weakly_canonical(myclass_path).string());
    CHECK(filter.should_include(myclass));
}

TEST_CASE("Test advanced diagram filter modules")
{
    auto cfg =
        clanguml::config::load("./test_config_data/filters_advanced.yml");

    auto &config = *cfg.diagrams["modules_test"];
    clanguml::include_diagram::model::diagram diagram;
    diagram.set_complete(true);

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

TEST_CASE("Test method_types include filter")
{
    auto cfg =
        clanguml::config::load("./test_config_data/filters_advanced.yml");

    auto &config = *cfg.diagrams["method_type_include_test"];
    clanguml::class_diagram::model::diagram diagram;
    diagram.set_complete(true);

    auto filter_ptr = diagram_filter_factory::create(diagram, config);
    diagram_filter &filter = *filter_ptr;

    class_method cm{access_t::kPublic, "A", ""};
    cm.is_constructor(true);

    CHECK(filter.should_include(cm));

    cm.is_constructor(false);
    cm.is_destructor(true);

    CHECK_FALSE(filter.should_include(cm));
}

TEST_CASE("Test elements and namespaces regexp filter")
{
    auto cfg =
        clanguml::config::load("./test_config_data/filters_advanced.yml");

    auto &config = *cfg.diagrams["regex_elements_and_namespaces"];
    clanguml::class_diagram::model::diagram diagram;

    auto filter_ptr = diagram_filter_factory::create(diagram, config);
    diagram_filter &filter = *filter_ptr;

    class_ c{{}};

    c.set_namespace(namespace_{"ns1"});
    c.set_name("ClassA");

    CHECK_FALSE(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns2"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns2"});
    c.set_name("ClassZ");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns5::ns3"});
    c.set_name("ClassA");

    CHECK_FALSE(filter.should_include(c));
}

TEST_CASE("Test edge filter and namespaces filter")
{
    auto cfg =
        clanguml::config::load("./test_config_data/filters_advanced.yml");

    auto &config = *cfg.diagrams["edge_filter_and_namespaces"];
    clanguml::class_diagram::model::diagram diagram;

    auto filter_ptr = diagram_filter_factory::create(diagram, config);
    diagram_filter &filter = *filter_ptr;

    diagram.set_complete(true);

    uint64_t id{1};

    {
        auto ns1 = std::make_unique<package>(namespace_{});
        ns1->set_name("ns1");
        ns1->set_id(eid_t{id++});
        diagram.add(namespace_{}, std::move(ns1));
    }
    {
        auto ns1__nsA = std::make_unique<package>(namespace_{});
        ns1__nsA->set_name("nsA");
        ns1__nsA->set_namespace(namespace_{"ns1"});
        ns1__nsA->set_id(eid_t{id++});
        diagram.add(namespace_{}, std::move(ns1__nsA));
    }
    {
        auto A = std::make_unique<class_>(namespace_{});
        A->set_namespace(namespace_{"ns1::nsA"});
        A->set_name("A");
        A->set_id(eid_t{id});
        diagram.add(namespace_{"ns1::nsA"}, std::move(A));
    }

    CHECK(filter.should_include(*diagram.get(eid_t{id})));

    class_ c{{}};
    c.set_namespace(namespace_{"ns2::nsB"});
    c.set_name("B");

    CHECK(filter.should_include(c));

    {
        auto C = std::make_unique<class_>(namespace_{});
        C->set_namespace(namespace_{"ns1::nsA"});
        C->set_name("C");
        C->set_id(eid_t{++id});
        diagram.add(namespace_{"ns1::nsA"}, std::move(C));
    }

    c.set_namespace(namespace_{"ns2::nsB"});
    c.set_name("C");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::nsA"});
    c.set_name("R");

    CHECK_FALSE(filter.should_include(c));
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
