/**
 * @file tests/test_cli_handler.cc
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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "cli/cli_handler.h"
#include "version.h"

#include "doctest/doctest.h"

#include <random>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> make_sstream_logger(std::ostream &ostr)
{
    auto oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(ostr);
    return std::make_shared<spdlog::logger>(
        "clanguml-logger", std::move(oss_sink));
}

std::string create_temp_file()
{
#ifdef _WIN32
    const std::string result = std::tmpnam(nullptr);
#else
    std::string result = std::filesystem::temp_directory_path();

    if (result.empty()) {
        result = "/tmp";
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib('A', 'Z');

    const size_t filename_length = 10;

    do {
        result += "/clanguml_test_";
        for (size_t i = 0; i < filename_length; ++i) {
            char random_char = static_cast<char>(distrib(gen));
            result += random_char;
        }
    } while (std::filesystem::exists(result));

#endif

    return result;
}

TEST_CASE("Test cli handler print_version")
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

TEST_CASE("Test cli handler print_config")
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

TEST_CASE("Test cli handler print_diagrams_list")
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

TEST_CASE("Test cli handler print_diagram_templates")
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

TEST_CASE("Test cli handler print_diagram_template")
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

TEST_CASE("Test cli handler print_diagram_template invalid template")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;

    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;

    std::vector<const char *> argv = {"clang-uml", "--config",
        "./test_config_data/simple.yml", "--show-template", "no_such_template"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kError);
}

TEST_CASE("Test cli handler properly adds new diagram configs from template")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;
    using clanguml::util::contains;

    std::string option_name;
    std::string diagram_name;

    // First create initial config file
    std::vector<const char *> argv{"clang-uml", "--init"};

    // Generate temporary file path
    const std::string config_file{create_temp_file()};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};
    cli.set_config_path(config_file);

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kExit);

    // Now add a template to the diagram
    std::vector<const char *> argv_add{
        "clang-uml",
        "--add-diagram-from-template",
        "class_context_tmpl",
        "--template-var",
        "diagram_name=A_context_diagram",
        "--template-var",
        "glob=test.cc",
        "--template-var",
        "using_namespace=ns1::ns2",
        "--template-var",
        "class_name=ns1::ns2::A",
        "--template-var",
        "namespace_name=ns1::ns2",
    };

    std::ostringstream ostr_add;

    cli_handler cli_add{ostr_add, make_sstream_logger(ostr_add)};
    cli_add.set_config_path(config_file);

    auto res_add = cli_add.handle_options(argv_add.size(), argv_add.data());
    REQUIRE(res_add == cli_flow_t::kExit);

    // Read contents of temp_file and check if they are valid
    std::ifstream ifs(config_file);
    std::string config_file_content{
        std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};

    REQUIRE(config_file_content ==
        R"(compilation_database_dir: .
output_directory: docs/diagrams
diagrams:
  example_class_diagram:
    type: class
    glob:
      - src/*.cpp
    using_namespace:
      - myproject
    include:
      namespaces:
        - myproject
    exclude:
      namespaces:
        - myproject::detail
  A_context_diagram:
    type: class
    glob: [test.cc]
    using_namespace: ns1::ns2
    include:
      context: [ns1::ns2::A]
      namespaces: [ns1::ns2]
)");
}

TEST_CASE("Test cli handler properly reports error when adding config from "
          "invalid template")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;
    using clanguml::util::contains;

    std::string option_name;
    std::string diagram_name;

    // First create initial config file
    std::vector<const char *> argv{"clang-uml", "--init"};

    // Generate temporary file path
    std::string config_file{create_temp_file()};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};
    cli.set_config_path(config_file);

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kExit);

    // Now add a template to the diagram
    std::vector<const char *> argv_add{
        "clang-uml",
        "--add-diagram-from-template",
        "invalid_template_name",
        "--template-var",
        "diagram_name=A_context_diagram",
        "--template-var",
        "glob=test.cc",
        "--template-var",
        "using_namespace=ns1::ns2",
        "--template-var",
        "class_name=ns1::ns2::A",
        "--template-var",
        "namespace_name=ns1::ns2",
    };

    std::ostringstream ostr_add;

    cli_handler cli_add{ostr_add, make_sstream_logger(ostr_add)};
    cli_add.set_config_path(config_file);

    auto res_add = cli_add.handle_options(argv_add.size(), argv_add.data());
    REQUIRE(res_add == cli_flow_t::kError);
}

TEST_CASE("Test cli handler add_compile_flag and remove_compile_flag")
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

TEST_CASE("Test cli handler puml config inheritance with render cmd")
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

TEST_CASE("Test cli handler properly initializes new config file")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;
    using clanguml::util::contains;

    std::vector<const char *> argv{"clang-uml", "--init"};

    // Generate temporary file path
    std::string config_file{create_temp_file()};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};
    cli.set_config_path(config_file);

    auto res = cli.handle_options(argv.size(), argv.data());

    // Read contents of temp_file and check if they are valid
    std::ifstream ifs(config_file);
    std::string config_file_content{
        std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};

    REQUIRE(config_file_content ==
        R"(# Change to directory where compile_commands.json is
compilation_database_dir: .
# Change to directory where diagram should be written
output_directory: docs/diagrams
diagrams:
  example_class_diagram:
    type: class
    glob:
      - src/*.cpp
    using_namespace:
      - myproject
    include:
      namespaces:
        - myproject
    exclude:
      namespaces:
        - myproject::detail
)");
}

TEST_CASE("Test cli handler properly adds new diagram configs")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;
    using clanguml::util::contains;

    std::string option_name;
    std::string diagram_name;

    SUBCASE("class")
    {
        option_name = "--add-class-diagram";
        diagram_name = "new_class_diagram";
    }
    SUBCASE("sequence")
    {
        option_name = "--add-sequence-diagram";
        diagram_name = "new_sequence_diagram";
    }
    SUBCASE("include")
    {
        option_name = "--add-include-diagram";
        diagram_name = "new_include_diagram";
    }
    SUBCASE("package")
    {
        option_name = "--add-package-diagram";
        diagram_name = "new_package_diagram";
    }

    CAPTURE(option_name);
    CAPTURE(diagram_name);

    std::vector<const char *> argv{"clang-uml", "--init"};

    // Generate temporary file path
    std::string config_file{create_temp_file()};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};
    cli.set_config_path(config_file);

    cli.handle_options(argv.size(), argv.data());

    std::vector<const char *> argv_add_diagram{
        "clang-uml", option_name.c_str(), diagram_name.c_str()};

    std::ostringstream ostr_add_diagram;
    cli_handler cli_add_diagram{
        ostr_add_diagram, make_sstream_logger(ostr_add_diagram)};
    cli_add_diagram.set_config_path(config_file);

    cli_add_diagram.handle_options(
        argv_add_diagram.size(), argv_add_diagram.data());

    // Read contents of temp_file and check if they are valid
    std::ifstream ifs(config_file);
    std::string config_file_content{
        std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};

    // Verify that the config_file YAML file contains 'new_class_diagram' object
    REQUIRE(clanguml::util::contains(
        config_file_content, fmt::format("{}:", diagram_name)));
}

TEST_CASE("Test cli handler fail to add diagram with stdin config")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;

    std::vector<const char *> argv = {"clang-uml", "--init", "-c", "-"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kError);
}

TEST_CASE("Test cli handler fail when print_to or print_from do not have "
          "specified diagram")
{
    using clanguml::cli::cli_flow_t;
    using clanguml::cli::cli_handler;

    std::vector<const char *> argv = {"clang-uml", "--print-from"};

    std::ostringstream ostr;
    cli_handler cli{ostr, make_sstream_logger(ostr)};

    auto res = cli.handle_options(argv.size(), argv.data());

    REQUIRE(res == cli_flow_t::kError);
}
