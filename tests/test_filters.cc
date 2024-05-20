/**
 * @file tests/test_filters.cc
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
#include "common/model/diagram_filter.h"
#include "common/model/source_file.h"
#include "config/config.h"
#include "include_diagram/model/diagram.h"
#include "sequence_diagram/model/diagram.h"

#include <filesystem>

TEST_CASE("Test diagram paths filter")
{
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::source_file;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["include_test"];
    clanguml::include_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

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

TEST_CASE("Test method_types include filter")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::source_file;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["method_type_include_test"];
    clanguml::class_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    class_method cm{access_t::kPublic, "A", ""};
    cm.is_constructor(true);

    CHECK(filter.should_include(cm));

    cm.is_constructor(false);
    cm.is_destructor(true);

    CHECK(!filter.should_include(cm));
}

TEST_CASE("Test method_types exclude filter")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::source_file;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["method_type_exclude_test"];
    clanguml::class_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    class_method cm{access_t::kPublic, "A", ""};

    CHECK(filter.should_include(cm));

    cm.is_constructor(true);

    CHECK(filter.should_include(cm));

    cm.is_constructor(false);
    cm.is_destructor(true);

    CHECK(!filter.should_include(cm));
}

TEST_CASE("Test namespaces filter")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::source_file;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["namespace_test"];
    clanguml::class_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    class_ c{{}};

    c.set_namespace(namespace_{"ns1::ns2"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns2::detail"});
    c.set_name("ClassAImpl");

    CHECK(!filter.should_include(c));

    c.set_namespace(namespace_{"ns1::interface"});
    c.set_name("IClassA");

    CHECK(!filter.should_include(c));

    CHECK(filter.should_include(namespace_{"ns1"}));
    CHECK(filter.should_include(namespace_{"ns1::ns2"}));
    CHECK(!filter.should_include(namespace_{"ns1::ns2::detail"}));
    CHECK(!filter.should_include(namespace_{"ns1::ns2::detail::more_detail"}));
    CHECK(!filter.should_include(namespace_{"ns1::interface"}));

    package p{{}};

    p.set_namespace({"ns1"});

    CHECK(filter.should_include(p));

    p.set_namespace({"ns1"});
    p.set_name("ns2");

    CHECK(filter.should_include(p));

    p.set_namespace({"ns1::ns2"});
    p.set_name("detail");

    CHECK(!filter.should_include(p));

    p.set_namespace({"ns1"});
    p.set_name("interface");

    CHECK(!filter.should_include(p));
}

TEST_CASE("Test elements regexp filter")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::source_file;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_elements_test"];
    clanguml::class_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    class_ c{{}};

    c.set_namespace(namespace_{"ns1"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns2"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns2"});
    c.set_name("ClassZ");

    CHECK(!filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns5::ns3"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));
}

TEST_CASE("Test namespaces regexp filter")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::source_file;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_namespace_test"];
    clanguml::class_diagram::model::diagram diagram;

    diagram_filter filter(diagram, config);

    class_ c{{}};

    c.set_namespace(namespace_{"ns1::ns2"});
    c.set_name("ClassA");

    CHECK(filter.should_include(c));

    c.set_namespace(namespace_{"ns1::ns2::detail"});
    c.set_name("ClassAImpl");

    CHECK(!filter.should_include(c));

    c.set_namespace(namespace_{"ns1::interface"});
    c.set_name("IClassA");

    CHECK(filter.should_include(c));

    CHECK(filter.should_include(namespace_{"ns1"}));
    CHECK(filter.should_include(namespace_{"ns1::ns2"}));
    CHECK(!filter.should_include(namespace_{"ns1::ns2::detail"}));
    CHECK(filter.should_include(namespace_{"ns1::interface"}));

    package p{{}};

    p.set_namespace({"ns1"});
    p.set_name("ns2");

    CHECK(filter.should_include(p));

    p.set_namespace({"ns1::ns2"});
    p.set_name("detail");

    CHECK(!filter.should_include(p));

    p.set_namespace({"ns1"});
    p.set_name("interface");

    CHECK(filter.should_include(p));
}

TEST_CASE("Test subclasses regexp filter")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::to_id;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::source_file;
    using namespace std::string_literals;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_subclasses_test"];
    clanguml::class_diagram::model::diagram diagram;

    auto p = std::make_unique<package>(config.using_namespace());
    p->set_namespace({});
    p->set_name("ns1");
    diagram.add({}, std::move(p));
    p = std::make_unique<package>(config.using_namespace());
    p->set_namespace({"ns1"});
    p->set_name("ns2");
    diagram.add(namespace_{"ns1"}, std::move(p));

    auto c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseA");
    c->set_id(to_id("ns1::ns2::BaseA"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("A1");
    c->set_id(to_id("ns1::ns2::A1"s));
    c->add_parent({"ns1::ns2::BaseA"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("A2");
    c->set_id(to_id("ns1::ns2::A2"s));
    c->add_parent({"ns1::ns2::BaseA"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseB");
    c->set_id(to_id("ns1::ns2::BaseB"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("B1");
    c->set_id(to_id("ns1::ns2::B1"s));
    c->add_parent({"ns1::ns2::BaseB"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("B2");
    c->set_id(to_id("ns1::ns2::B2"s));
    c->add_parent({"ns1::ns2::BaseB"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("Common");
    c->set_id(to_id("ns1::ns2::Common"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("C1");
    c->set_id(to_id("ns1::ns2::C1"s));
    c->add_parent({"ns1::ns2::Common"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    diagram.set_complete(true);

    diagram_filter filter(diagram, config);

    CHECK(filter.should_include(*diagram.find<class_>("ns1::ns2::A1")));
    CHECK(filter.should_include(*diagram.find<class_>("ns1::ns2::B1")));
    CHECK(!filter.should_include(*diagram.find<class_>("ns1::ns2::C1")));
}

TEST_CASE("Test parents regexp filter")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::to_id;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::source_file;
    using namespace std::string_literals;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_parents_test"];
    clanguml::class_diagram::model::diagram diagram;

    auto p = std::make_unique<package>(config.using_namespace());
    p->set_namespace({});
    p->set_name("ns1");
    diagram.add({}, std::move(p));
    p = std::make_unique<package>(config.using_namespace());
    p->set_namespace({"ns1"});
    p->set_name("ns2");
    diagram.add(namespace_{"ns1"}, std::move(p));

    auto c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseA");
    c->set_id(to_id("ns1::ns2::BaseA"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("A1");
    c->set_id(to_id("ns1::ns2::A1"s));
    c->add_parent({"ns1::ns2::BaseA"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("A2");
    c->set_id(to_id("ns1::ns2::A2"s));
    c->add_parent({"ns1::ns2::BaseA"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("BaseB");
    c->set_id(to_id("ns1::ns2::BaseB"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("B1");
    c->set_id(to_id("ns1::ns2::B1"s));
    c->add_parent({"ns1::ns2::BaseB"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("B2");
    c->set_id(to_id("ns1::ns2::B2"s));
    c->add_parent({"ns1::ns2::BaseB"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("Common");
    c->set_id(to_id("ns1::ns2::Common"s));
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_name("C3");
    c->set_id(to_id("ns1::ns2::C3"s));
    c->add_parent({"ns1::ns2::Common"});
    diagram.add(namespace_{"ns1::ns2"}, std::move(c));

    diagram.set_complete(true);

    diagram_filter filter(diagram, config);

    CHECK(filter.should_include(*diagram.find<class_>("ns1::ns2::BaseA")));
    CHECK(filter.should_include(*diagram.find<class_>("ns1::ns2::BaseB")));
    CHECK(!filter.should_include(*diagram.find<class_>("ns1::ns2::Common")));
}

TEST_CASE("Test specializations regexp filter")
{
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::to_id;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::relationship;
    using clanguml::common::model::relationship_t;
    using clanguml::common::model::source_file;
    using clanguml::common::model::template_parameter;
    using namespace std::string_literals;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_specializations_test"];
    clanguml::class_diagram::model::diagram diagram;

    const auto template_id = to_id("A<Ts...>"s);

    auto c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A");
    c->add_template(
        template_parameter::make_template_type("T", std::nullopt, true));
    c->set_id(template_id);
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A");
    c->add_template(template_parameter::make_argument("double"));
    c->set_id(to_id("A<double>"s));
    c->add_relationship(
        relationship{relationship_t::kInstantiation, template_id});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A");
    c->add_template(template_parameter::make_argument("int"));
    c->set_id(to_id("A<int>"s));
    c->add_relationship(
        relationship{relationship_t::kInstantiation, template_id});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A");
    c->add_template(template_parameter::make_argument("int"));
    c->add_template(template_parameter::make_argument("std::string"));
    c->set_id(to_id("A<int,std::string>"s));
    c->add_relationship(
        relationship{relationship_t::kInstantiation, template_id});
    diagram.add(namespace_{}, std::move(c));

    diagram.set_complete(true);

    diagram_filter filter(diagram, config);

    CHECK(filter.should_include(*diagram.find<class_>("A<int,std::string>")));
    CHECK(!filter.should_include(*diagram.find<class_>("A<double>")));
}

TEST_CASE("Test context regexp filter")
{
    using clanguml::class_diagram::model::class_;
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::to_id;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::relationship;
    using clanguml::common::model::relationship_t;
    using clanguml::common::model::source_file;
    using clanguml::common::model::template_parameter;
    using namespace std::string_literals;

    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_context_test"];
    clanguml::class_diagram::model::diagram diagram;

    auto c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A");
    c->set_id(to_id("A"s));
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A1");
    c->set_id(to_id("A1"s));
    c->add_relationship(
        relationship{relationship_t::kAssociation, to_id("A"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A2");
    c->set_id(to_id("A2"s));
    c->add_relationship(relationship{relationship_t::kDependency, to_id("A"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A21");
    c->set_id(to_id("A21"s));
    c->add_relationship(
        relationship{relationship_t::kDependency, to_id("A2"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("B");
    c->set_id(to_id("B"s));
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("B1");
    c->set_id(to_id("B1"s));
    c->add_relationship(
        relationship{relationship_t::kAssociation, to_id("B"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("C");
    c->set_id(to_id("C"s));
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("C1");
    c->set_id(to_id("C1"s));
    c->add_relationship(
        relationship{relationship_t::kAssociation, to_id("C"s)});
    diagram.add(namespace_{}, std::move(c));

    diagram.set_complete(true);

    diagram_filter filter(diagram, config);

    CHECK(filter.should_include(*diagram.find<class_>("A")));
    CHECK(filter.should_include(*diagram.find<class_>("A1")));
    CHECK(filter.should_include(*diagram.find<class_>("A2")));

    CHECK(!filter.should_include(*diagram.find<class_>("A21")));

    CHECK(filter.should_include(*diagram.find<class_>("B")));
    CHECK(filter.should_include(*diagram.find<class_>("B1")));

    CHECK(!filter.should_include(*diagram.find<class_>("C")));
    CHECK(!filter.should_include(*diagram.find<class_>("C1")));
}

TEST_CASE("Test dependencies regexp filter")
{
    using clanguml::class_diagram::model::class_;
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::to_id;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::relationship;
    using clanguml::common::model::relationship_t;
    using clanguml::common::model::source_file;
    using clanguml::common::model::template_parameter;
    using namespace std::string_literals;
    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_dependencies_test"];
    clanguml::class_diagram::model::diagram diagram;

    auto c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A");
    c->set_id(to_id("A"s));
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A1");
    c->set_id(to_id("A1"s));
    c->add_relationship(relationship{relationship_t::kDependency, to_id("A"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A2");
    c->set_id(to_id("A2"s));
    c->add_relationship(relationship{relationship_t::kDependency, to_id("A"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A21");
    c->set_id(to_id("A21"s));
    c->add_relationship(
        relationship{relationship_t::kDependency, to_id("A2"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("B");
    c->set_id(to_id("B"s));
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("B1");
    c->set_id(to_id("B1"s));
    c->add_relationship(relationship{relationship_t::kDependency, to_id("B"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("C");
    c->set_id(to_id("C"s));
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("C1");
    c->set_id(to_id("C1"s));
    c->add_relationship(relationship{relationship_t::kDependency, to_id("C"s)});
    diagram.add(namespace_{}, std::move(c));

    diagram.set_complete(true);

    diagram_filter filter(diagram, config);

    CHECK(filter.should_include(*diagram.find<class_>("A")));
    CHECK(!filter.should_include(*diagram.find<class_>("A1")));
    CHECK(filter.should_include(*diagram.find<class_>("A2")));
    CHECK(filter.should_include(*diagram.find<class_>("A21")));

    CHECK(filter.should_include(*diagram.find<class_>("B")));
    CHECK(filter.should_include(*diagram.find<class_>("B1")));

    CHECK(!filter.should_include(*diagram.find<class_>("C")));
    CHECK(!filter.should_include(*diagram.find<class_>("C1")));
}

TEST_CASE("Test dependants regexp filter")
{
    using clanguml::class_diagram::model::class_;
    using clanguml::class_diagram::model::class_method;
    using clanguml::class_diagram::model::class_parent;
    using clanguml::common::to_id;
    using clanguml::common::model::access_t;
    using clanguml::common::model::diagram_filter;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::relationship;
    using clanguml::common::model::relationship_t;
    using clanguml::common::model::source_file;
    using clanguml::common::model::template_parameter;
    using namespace std::string_literals;
    using clanguml::class_diagram::model::class_;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["regex_dependants_test"];
    clanguml::class_diagram::model::diagram diagram;

    auto c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A");
    c->set_id(to_id("A"s));
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A1");
    c->set_id(to_id("A1"s));
    c->add_relationship(relationship{relationship_t::kDependency, to_id("A"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A2");
    c->set_id(to_id("A2"s));
    c->add_relationship(relationship{relationship_t::kDependency, to_id("A"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("A21");
    c->set_id(to_id("A21"s));
    c->add_relationship(
        relationship{relationship_t::kDependency, to_id("A2"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("B");
    c->set_id(to_id("B"s));
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("B1");
    c->set_id(to_id("B1"s));
    c->add_relationship(relationship{relationship_t::kDependency, to_id("B"s)});
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("C");
    c->set_id(to_id("C"s));
    diagram.add(namespace_{}, std::move(c));

    c = std::make_unique<class_>(config.using_namespace());
    c->set_name("C1");
    c->set_id(to_id("C1"s));
    c->add_relationship(relationship{relationship_t::kDependency, to_id("C"s)});
    diagram.add(namespace_{}, std::move(c));

    diagram.set_complete(true);

    diagram_filter filter(diagram, config);

    CHECK(filter.should_include(*diagram.find<class_>("A")));
    CHECK(filter.should_include(*diagram.find<class_>("A1")));
    CHECK(filter.should_include(*diagram.find<class_>("A2")));
    CHECK(filter.should_include(*diagram.find<class_>("A21")));

    CHECK(filter.should_include(*diagram.find<class_>("B")));
    CHECK(filter.should_include(*diagram.find<class_>("B1")));

    CHECK(!filter.should_include(*diagram.find<class_>("C")));
    CHECK(!filter.should_include(*diagram.find<class_>("C1")));
}

TEST_CASE("Test callee_types filter")
{
    using clanguml::common::to_id;
    using clanguml::common::model::diagram_filter;
    using clanguml::sequence_diagram::model::class_;
    using clanguml::sequence_diagram::model::function;
    using clanguml::sequence_diagram::model::function_template;
    using clanguml::sequence_diagram::model::method;
    using clanguml::sequence_diagram::model::participant;

    using namespace std::string_literals;

    auto cfg = clanguml::config::load("./test_config_data/filters.yml");

    auto &config = *cfg.diagrams["callee_type_include_test"];
    clanguml::sequence_diagram::model::diagram diagram;

    std::unique_ptr<participant> p;

    p = std::make_unique<function>(config.using_namespace());
    p->set_name("A");
    p->set_id(to_id("A"s));
    diagram.add_participant(std::move(p));

    p = std::make_unique<function_template>(config.using_namespace());
    p->set_name("A1");
    p->set_id(to_id("A1"s));
    diagram.add_participant(std::move(p));

    p = std::make_unique<class_>(config.using_namespace());
    p->set_name("C1");
    p->set_id(to_id("C1"s));
    diagram.add_participant(std::move(p));

    p = std::make_unique<method>(config.using_namespace());
    p->set_name("M1");
    p->set_id(to_id("M1"s));
    dynamic_cast<method *>(p.get())->set_class_id(to_id("C1"s));
    diagram.add_participant(std::move(p));

    diagram.set_complete(true);
    diagram_filter filter(diagram, config);

    CHECK(
        filter.should_include(*diagram.get_participant<function>(to_id("A"s))));
    CHECK(filter.should_include(
        *diagram.get_participant<function_template>(to_id("A1"s))));
    CHECK(!filter.should_include(
        *diagram.get_participant<participant>(to_id("M1"s))));
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
