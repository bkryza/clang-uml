/**
 * tests/test_cli_handler.cc
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