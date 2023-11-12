/**
 * @file tests/test_cli_handler.cc
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

#include "cli/cli_handler.h"
#include "version.h"

#include "catch.h"

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> make_sstream_logger(std::ostream &ostr)
{
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(ostr);
    return std::make_shared<spdlog::logger>(
        "clanguml-logger", std::move(oss_sink));
}

TEST_CASE("Test cli handler print_version", "[unit-test]")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;

    std::vector<const char *> argv = {"clang-uml", "--version"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kExit);

    auto output = ostr.str();

    REQUIRE(output.find(fmt::format(
                "clang-uml {}", clanguml::version::CLANG_UML_VERSION)) == 0);
}

TEST_CASE("Test cli handler print_config", "[unit-test]")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;

    std::vector<const char *> argv = {"clang-uml", "--config",
        "./test_config_data/simple.yml", "--dump-config"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kExit);

    auto output = ostr.str();
    YAML::Node doc = YAML::Load(output);

    REQUIRE(doc["diagrams"]["class_main"]);
}

TEST_CASE("Test cli handler print_diagrams_list", "[unit-test]")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;

    std::vector<const char *> argv = {
        "clang-uml", "--config", "./test_config_data/simple.yml", "-l"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kExit);

    auto output = ostr.str();

    REQUIRE(output ==
        "The following diagrams are defined in the config file:"
        R"(
  - class_main [class]
)");
}

TEST_CASE("Test cli handler print_diagram_templates", "[unit-test]")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;

    std::vector<const char *> argv = {"clang-uml", "--config",
        "./test_config_data/simple.yml", "--list-templates"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kExit);

    auto output = ostr.str();

    REQUIRE(output ==
        "The following diagram templates are available:"
        R"(
  - class_context_tmpl [class]: Generate class context diagram
  - parents_hierarchy_tmpl [class]: Generate class parents inheritance diagram
  - subclass_hierarchy_tmpl [class]: Generate class children inheritance diagram
)");
}

TEST_CASE("Test cli handler print_diagram_template", "[unit-test]")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;

    std::vector<const char *> argv = {"clang-uml", "--config",
        "./test_config_data/simple.yml", "--show-template",
        "class_context_tmpl"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kExit);

    auto output = ostr.str();

    REQUIRE(output ==
        "\"{{ diagram_name }}\":"
        R"(
  type: class
  {% if exists("glob") %}
  glob: [{{ glob }}]
  {% endif %}
  {% if exists("using_namespace") %}
  using_namespace: {{ using_namespace }}
  {% endif %}
  include:
    context: [{{ class_name }}]
    namespaces: [{{ namespace_name }}]

)");
}

TEST_CASE(
    "Test cli handler add_compile_flag and remove_compile_flag", "[unit-test]")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;
    using clanguml::util::contains;

    std::vector<const char *> argv{"clang-uml", "--config",
        "./test_config_data/simple.yml", "--add-compile-flag", "-Wno-error",
        "--add-compile-flag", "-Wno-warning", "--remove-compile-flag",
        "-I/usr/include"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kContinue);

    REQUIRE(cli.config.add_compile_flags.has_value);
    REQUIRE(cli.config.remove_compile_flags.has_value);

    REQUIRE(contains(cli.config.add_compile_flags(), "-Wno-error"));
    REQUIRE(contains(cli.config.add_compile_flags(), "-Wno-warning"));
    REQUIRE(contains(cli.config.remove_compile_flags(), "-I/usr/include"));
}

TEST_CASE(
    "Test cli handler puml config inheritance with render cmd", "[unit-test]")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;
    using clanguml::util::contains;

    std::vector<const char *> argv{"clang-uml", "--config",
        "./test_config_data/render_cmd.yml", "-r",
        "--mermaid-cmd=mmdc -i output/{}.mmd -o output/{}.svg"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kContinue);

    REQUIRE(contains(cli.get_runtime_config().output_directory, "output"));
    REQUIRE(cli.get_runtime_config().render_diagrams);
    REQUIRE(cli.config.diagrams.at("class_main")->puml().cmd ==
        "plantuml -tsvg output/{}.puml");
    REQUIRE(cli.config.diagrams.at("class_main")->mermaid().cmd ==
        "mmdc -i output/{}.mmd -o output/{}.svg");
    REQUIRE(cli.config.diagrams.at("class_main")->puml().after.at(0) ==
        "' test comment");
}