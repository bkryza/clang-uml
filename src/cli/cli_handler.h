/**
 * src/options/cli_handler.h
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
#pragma once

#include "common/model/enums.h"
#include "config/config.h"

#include <cli11/CLI11.hpp>

#include <optional>

namespace clanguml::cli {

enum class cli_flow_t { kExit, kError, kContinue };

class cli_handler {
public:
    cli_handler(std::ostream &ostr = std::cout,
        std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt(
            "clanguml-logger", spdlog::color_mode::automatic));

    /**
     * Main CLI handling method.
     *
     * @param argc
     * @param argv
     * @return
     */
    cli_flow_t handle_options(int argc, const char **argv);

    /**
     * Print the program version and basic information
     */
    cli_flow_t print_version();

    /**
     * Print list of diagrams available in the configuration file
     */
    cli_flow_t print_diagrams_list();

    /**
     * Print list of available diagram templates, including their names
     * and types.
     */
    cli_flow_t print_diagram_templates();

    /**
     * Print definition of a specific diagram template.
     *
     * @param template_name
     * @return
     */
    cli_flow_t print_diagram_template(const std::string &template_name);

    /**
     * Print effective config after loading and setting default values.
     */
    cli_flow_t print_config();

    /**
     * Generate sample configuration file and exit.
     *
     * @return 0 on success or error code
     */
    cli_flow_t create_config_file();

    /**
     * Add example diagram of given type to the config file.
     *
     * @param type Type of the sample diagram to add
     * @param config_file_path Path to the config file
     * @param name Name of the new diagram
     * @return 0 on success or error code
     */
    cli_flow_t add_config_diagram(clanguml::common::model::diagram_t type,
        const std::string &config_file_path, const std::string &name);

    /**
     * Add diagram based on template
     *
     * @param config_file_path
     * @param template_name
     * @param template_variables
     * @return
     */
    cli_flow_t add_config_diagram_from_template(
        const std::string &config_file_path, const std::string &template_name,
        const std::vector<std::string> &template_variables);

    /**
     * Check if diagram output directory exists, if not create it
     *
     * @param dir Path to the output directory
     * @return True if directory exists or has been created
     */
    bool ensure_output_directory_exists(const std::string &dir);

    std::string config_path{".clang-uml"};
    std::optional<std::string> compilation_database_dir{};
    std::vector<std::string> diagram_names{};
    std::optional<std::string> output_directory{};
    std::string effective_output_directory{};
    unsigned int thread_count{};
    bool show_version{false};
    int verbose{};
    bool list_diagrams{false};
    bool quiet{false};
    bool initialize{false};
    std::optional<std::vector<std::string>> add_compile_flag;
    std::optional<std::vector<std::string>> remove_compile_flag;
    std::optional<std::string> add_class_diagram;
    std::optional<std::string> add_sequence_diagram;
    std::optional<std::string> add_package_diagram;
    std::optional<std::string> add_include_diagram;
    std::optional<std::string> add_diagram_from_template;
    bool dump_config{false};
    std::optional<bool> paths_relative_to_pwd{};
    std::vector<std::string> template_variables{};
    bool list_templates{false};
    std::optional<bool> no_metadata{};
    std::optional<std::string> show_template;
    std::vector<clanguml::common::generator_type_t> generators{
        clanguml::common::generator_type_t::plantuml};

    clanguml::config::config config;

private:
    cli_flow_t parse(int argc, const char **argv);

    cli_flow_t handle_pre_config_options();

    cli_flow_t load_config();

    cli_flow_t handle_post_config_options();

    void setup_logging();

    std::ostream &ostr_;
    std::shared_ptr<spdlog::logger> logger_;
    CLI::App app{"Clang-based UML diagram generator for C++"};
};
} // namespace clanguml::cli