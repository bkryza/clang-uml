/**
 * tests/test_config.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
    using clanguml::common::model::access_t;
    using clanguml::common::model::relationship_t;
    using clanguml::util::contains;

    auto cfg = clanguml::config::load("./test_config_data/simple.yml");

    CHECK(cfg.diagrams.size() == 1);
    auto &diagram = *cfg.diagrams["class_main"];
    CHECK(diagram.type() == clanguml::common::model::diagram_t::kClass);
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

    CHECK(
        diagram.comment_parser() == clanguml::config::comment_parser_t::clang);

    CHECK(contains(diagram.include().access, access_t::kPublic));
    CHECK(contains(diagram.include().access, access_t::kProtected));
    CHECK(contains(diagram.include().access, access_t::kPrivate));

    CHECK(
        contains(diagram.include().relationships, relationship_t::kExtension));
    CHECK(contains(
        diagram.include().relationships, relationship_t::kAggregation));
    CHECK(contains(
        diagram.include().relationships, relationship_t::kAssociation));
    CHECK(contains(
        diagram.include().relationships, relationship_t::kComposition));
    CHECK(contains(
        diagram.include().relationships, relationship_t::kContainment));
    CHECK(
        contains(diagram.include().relationships, relationship_t::kDependency));
    CHECK(
        contains(diagram.include().relationships, relationship_t::kFriendship));
    CHECK(contains(
        diagram.include().relationships, relationship_t::kInstantiation));
    CHECK(
        contains(diagram.include().relationships, relationship_t::kOwnership));

    CHECK(contains(diagram.exclude().relationships, relationship_t::kNone));

    CHECK(diagram.relationship_hints().at("std::vector").get(0) ==
        relationship_t::kComposition);
    CHECK(diagram.relationship_hints().at("std::tuple").get(10) ==
        relationship_t::kAggregation);
    CHECK(diagram.relationship_hints().at("std::map").get(0) ==
        relationship_t::kNone);
    CHECK(diagram.relationship_hints().at("std::map").get(1) ==
        relationship_t::kComposition);
    CHECK(diagram.relationship_hints().at("std::shared_ptr").get(0) ==
        relationship_t::kAssociation);
    CHECK(diagram.relationship_hints().at("std::weak_ptr").get(0) ==
        relationship_t::kAssociation);
    CHECK(diagram.relationship_hints().at("std::unique_ptr").get(0) ==
        relationship_t::kAggregation);
    CHECK(diagram.relationship_hints().at("ns1::n2::some_template").get(0) ==
        relationship_t::kAssociation);
    CHECK(diagram.relationship_hints().at("ns1::n2::some_template").get(2) ==
        relationship_t::kComposition);
    CHECK(diagram.relationship_hints().at("ns1::n2::some_template").get(10) ==
        relationship_t::kAggregation);
}

TEST_CASE("Test config inherited", "[unit-test]")
{
    auto cfg = clanguml::config::load("./test_config_data/inherited.yml");

    CHECK(cfg.diagrams.size() == 2);
    auto &def = *cfg.diagrams["class_default"];
    CHECK(def.type() == clanguml::common::model::diagram_t::kClass);
    CHECK(def.glob().size() == 2);
    CHECK(def.glob()[0] == "src/**/*.cc");
    CHECK(def.glob()[1] == "src/**/*.h");
    CHECK(clanguml::util::contains(def.using_namespace(), "clanguml"));
    CHECK(def.generate_packages() == false);
    CHECK(def.generate_links == false);

    auto &cus = *cfg.diagrams["class_custom"];
    CHECK(cus.type() == clanguml::common::model::diagram_t::kClass);
    CHECK(cus.glob().size() == 1);
    CHECK(cus.glob()[0] == "src/main.cc");
    CHECK(cus.using_namespace().starts_with({"clanguml::ns1"}));
    CHECK(cus.include_relations_also_as_members());
    CHECK(cus.generate_packages() == false);
    CHECK(cus.generate_links == false);
    CHECK(cus.puml().before.size() == 2);
    CHECK(cus.puml().before.at(0) == "title This is diagram A");
    CHECK(cus.puml().before.at(1) == "This is a common header");
    CHECK(cus.puml().after.size() == 2);
    CHECK(cus.puml().after.at(0) == "note left of A: This is a note");
    CHECK(cus.puml().after.at(1) == "This is a common footnote");
}

TEST_CASE("Test config includes", "[unit-test]")
{
    auto cfg = clanguml::config::load("./test_config_data/includes.yml");

    CHECK(cfg.diagrams.size() == 2);
    auto &def = *cfg.diagrams["class_1"];
    CHECK(def.type() == clanguml::common::model::diagram_t::kClass);
    CHECK(def.glob().size() == 2);
    CHECK(def.glob()[0] == "src/**/*.cc");
    CHECK(def.glob()[1] == "src/**/*.h");
    CHECK(clanguml::util::contains(def.using_namespace(), "clanguml"));
    CHECK(def.generate_method_arguments() ==
        clanguml::config::method_arguments::none);

    auto &cus = *cfg.diagrams["class_2"];
    CHECK(cus.type() == clanguml::common::model::diagram_t::kClass);
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

    [[maybe_unused]] auto &def = static_cast<clanguml::config::class_diagram &>(
        *cfg.diagrams["class_main"]);

    auto check_layout = [](const auto &diagram,
                            const clanguml::common::model::diagram_t type) {
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
        clanguml::common::model::diagram_t::kClass);

    check_layout(static_cast<clanguml::config::package_diagram &>(
                     *cfg.diagrams["package_main"]),
        clanguml::common::model::diagram_t::kPackage);
}

TEST_CASE("Test config emitters", "[unit-test]")
{
    auto cfg = clanguml::config::load("./test_config_data/complete.yml");

    YAML::Emitter out;
    out.SetIndent(2);

    out << cfg;
    out << YAML::Newline;

    // Write the emitted YAML to a temp file
    auto tmp_file = std::filesystem::temp_directory_path() /
        fmt::format("clang-uml-{:016}", rand());

    {
        std::ofstream stream(tmp_file.string().c_str(), std::ios::binary);
        stream << out.c_str();
    }

    auto cfg_emitted = clanguml::config::load(tmp_file.string());

    REQUIRE(cfg.diagrams.size() == cfg_emitted.diagrams.size());

    std::filesystem::remove(tmp_file);
}