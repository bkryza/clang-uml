/**
 * tests/test_config.cc
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

#include "config/config.h"
#include "util/util.h"

TEST_CASE("Test config simple", "[unit-test]")
{
    auto cfg = clanguml::config::load("./test_config_data/simple.yml");

    CHECK(cfg.diagrams.size() == 1);
    auto &diagram = *cfg.diagrams["class_main"];
    CHECK(diagram.type() == clanguml::config::diagram_type::class_diagram);
    CHECK(diagram.glob().size() == 2);
    CHECK(clanguml::util::contains(diagram.using_namespace(), "clanguml"));
    CHECK(diagram.generate_method_arguments() ==
        clanguml::config::method_arguments::full);
    CHECK(diagram.generate_packages() == true);
    CHECK(diagram.generate_links == true);
    CHECK(diagram.generate_links().link ==
        "https://github.com/bkryza/clang-uml/blob/{{ git.branch }}/{{ "
        "element.source.file }}#L{{ element.source.line }}");
    CHECK(diagram.generate_links().tooltip == "{{ element.comment }}");
}

TEST_CASE("Test config inherited", "[unit-test]")
{
    auto cfg = clanguml::config::load("./test_config_data/inherited.yml");

    CHECK(cfg.diagrams.size() == 2);
    auto &def = *cfg.diagrams["class_default"];
    CHECK(def.type() == clanguml::config::diagram_type::class_diagram);
    CHECK(def.glob().size() == 2);
    CHECK(def.glob()[0] == "src/**/*.cc");
    CHECK(def.glob()[1] == "src/**/*.h");
    CHECK(clanguml::util::contains(def.using_namespace(), "clanguml"));
    CHECK(def.generate_packages() == false);
    CHECK(def.generate_links == false);

    auto &cus = *cfg.diagrams["class_custom"];
    CHECK(cus.type() == clanguml::config::diagram_type::class_diagram);
    CHECK(cus.glob().size() == 1);
    CHECK(cus.glob()[0] == "src/main.cc");
    CHECK(cus.using_namespace().starts_with({"clanguml::ns1"}));
    CHECK(cus.include_relations_also_as_members());
    CHECK(cus.generate_packages() == false);
    CHECK(cus.generate_links == false);
}

TEST_CASE("Test config includes", "[unit-test]")
{
    auto cfg = clanguml::config::load("./test_config_data/includes.yml");

    CHECK(cfg.diagrams.size() == 2);
    auto &def = *cfg.diagrams["class_1"];
    CHECK(def.type() == clanguml::config::diagram_type::class_diagram);
    CHECK(def.glob().size() == 2);
    CHECK(def.glob()[0] == "src/**/*.cc");
    CHECK(def.glob()[1] == "src/**/*.h");
    CHECK(clanguml::util::contains(def.using_namespace(), "clanguml"));
    CHECK(def.generate_method_arguments() ==
        clanguml::config::method_arguments::none);

    auto &cus = *cfg.diagrams["class_2"];
    CHECK(cus.type() == clanguml::config::diagram_type::class_diagram);
    CHECK(cus.glob().size() == 1);
    CHECK(cus.glob()[0] == "src/main.cc");
    CHECK(cus.using_namespace().starts_with({"clanguml::ns1"}));
    CHECK(cus.include_relations_also_as_members());
    CHECK(cus.generate_method_arguments() ==
        clanguml::config::method_arguments::none);
}

TEST_CASE("Test config layout", "[unit-test]")
{
    auto cfg = clanguml::config::load("./test_config_data/layout.yml");

    CHECK(cfg.diagrams.size() == 2);

    auto &def = static_cast<clanguml::config::class_diagram &>(
        *cfg.diagrams["class_main"]);

    auto check_layout = [](const auto &diagram,
                            const clanguml::config::diagram_type type) {
        CHECK(diagram.type() == type);

        CHECK(diagram.layout().at("ABCD").size() == 2);
        CHECK(diagram.layout().at("ABCD")[0].hint ==
            clanguml::config::hint_t::up);
        CHECK(diagram.layout().at("ABCD")[0].entity == "ABCD_SUBCLASS");
        CHECK(diagram.layout().at("ABCD")[1].hint ==
            clanguml::config::hint_t::left);
        CHECK(diagram.layout().at("ABCD")[1].entity == "ABCD_SIBLING");

        CHECK(diagram.layout().at("ABCD_SIBLING").size() == 2);
        CHECK(diagram.layout().at("ABCD_SIBLING")[0].hint ==
            clanguml::config::hint_t::right);
        CHECK(diagram.layout().at("ABCD_SIBLING")[0].entity ==
            "ABCD_OTHER_SIBLING");
        CHECK(diagram.layout().at("ABCD_SIBLING")[1].hint ==
            clanguml::config::hint_t::down);
        CHECK(diagram.layout().at("ABCD_SIBLING")[1].entity ==
            "ABCD_SIBLING_SIBLING");
    };

    check_layout(static_cast<clanguml::config::class_diagram &>(
                     *cfg.diagrams["class_main"]),
        clanguml::config::diagram_type::class_diagram);

    check_layout(static_cast<clanguml::config::package_diagram &>(
                     *cfg.diagrams["package_main"]),
        clanguml::config::diagram_type::package_diagram);
}