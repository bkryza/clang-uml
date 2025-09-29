/**
 * @file tests/test_diagram_append.cc
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

#include "cli/cli_handler.h"

#include "class_diagram/model/diagram.h"

auto id()
{
    using clanguml::common::model::eid_t;

    static uint64_t id_counter{1UL};

    return eid_t{id_counter++};
}

TEST_CASE("Test class diagram append")
{
    using clanguml::class_diagram::model::class_;
    using clanguml::class_diagram::model::diagram;
    using clanguml::class_diagram::model::enum_;
    using clanguml::common::model::eid_t;
    using clanguml::common::model::namespace_;
    using clanguml::common::model::package;
    using clanguml::common::model::relationship;
    using clanguml::common::model::relationship_t;
    using clanguml::config::class_diagram;

    class_diagram cfg;

    diagram d1{cfg};
    d1.set_name("test");

    auto un = namespace_{};

    auto ns1 = std::make_unique<package>(un);
    ns1->set_name("ns1");
    ns1->set_id(id());
    d1.add(namespace_{}, std::move(ns1));

    auto c1 = std::make_unique<class_>(un);
    c1->set_id(id());
    const auto c1_id = c1->id();
    c1->set_name("c1");
    c1->set_namespace(namespace_{"ns1"});
    d1.add_class(std::move(c1));
    d1.set_complete(true);

    diagram d2{cfg};
    d2.set_name("test");

    ns1 = std::make_unique<package>(un);
    ns1->set_name("ns1");
    ns1->set_id(id());
    d2.add(namespace_{}, std::move(ns1));

    auto c2 = std::make_unique<class_>(un);
    c2->set_id(id());
    const auto c2_id = c2->id();
    c2->set_name("c2");
    c2->set_namespace(namespace_{"ns1"});
    d2.add_class(std::move(c2));
    d2.set_complete(true);

    d2.add_relationship(
        relationship{relationship_t::kDependency, c1_id, c2_id});

    d1.append(std::move(d2));

    CHECK_FALSE(d1.is_empty());
    CHECK(d1.get(c1_id).has_value());
    CHECK(d1.get(c2_id).has_value());
    CHECK(d1.relationships().size() == 1);
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
