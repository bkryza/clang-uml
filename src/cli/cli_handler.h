/**
 * @file src/options/cli_handler.h
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
#pragma once

#include "common/model/enums.h"
#include "config/config.h"

#include <cli11/CLI11.hpp>

#include <optional>

namespace clanguml::cli {

/**
 * @brief This class holds command line parameters not directly related to
 *        specific diagram configurations.
 */
struct runtime_config {
    int verbose{};
    std::vector<clanguml::common::generator_type_t> generators{};
    bool print_from{};
    bool print_to{};
    bool progress{};
    unsigned int thread_count{};
    bool render_diagrams{};
    std::string output_directory{};
};

/**
 * This enum represents possible exit states of the command line parser.
 */
enum class cli_flow_t {
    kExit,    /*!< The application should exit (e.g. `-h`) */
    kError,   /*!< The options or configuration file were invalid */
    kContinue /*!< Continue with processing diagrams */
};

/**
 * @brief Command line options handler
 *
 * This class is responsible for handling the command line options
 * and executing required actions.
 */
class cli_handler {
public:
    cli_handler(std::ostream &ostr = std::cout,
        std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt(
            "clanguml-logger", spdlog::color_mode::automatic));

    /**
     * Main CLI handling method.
     *
     * @embed{cli_handle_options_sequence.svg}
     *
     * @param argc
     * @param argv
     * @return Command line handler state
     */
    cli_flow_t handle_options(int argc, const char **argv);

    /**
     * Print the program version and basic information
     *
     * @return Command line handler state
     */
    cli_flow_t print_version();

    /**
     * Print list of diagrams available in the configuration file
     *
     * @return Command line handler state
     */
    cli_flow_t print_diagrams_list();

    /**
     * Print list of available diagram templates, including their names
     * and types.
     *
     * @return Command line handler state
     */
    cli_flow_t print_diagram_templates();

    /**
     * Print definition of a specific diagram template.
     *
     * @param template_name Name of the diagram template
     * @return Command line handler state
     */
    cli_flow_t print_diagram_template(const std::string &template_name);

    /**
     * Print effective config after loading and setting default values.
     *
     * @return Command line handler state
     */
    cli_flow_t print_config();

    /**
     * Generate sample configuration file and exit.
     *
     * @return Command line handler state
     */
    cli_flow_t create_config_file();

    /**
     * Add example diagram of given type to the config file.
     *
     * @param type Type of the sample diagram to add
     * @param config_file_path Path to the config file
     * @param name Name of the new diagram
     * @return Command line handler state
     */
    cli_flow_t add_config_diagram(clanguml::common::model::diagram_t type,
        const std::string &config_file_path, const std::string &name);

    /**
     * Add diagram based on template
     *
     * @param config_file_path Path to the configuration file
     * @param template_name Name of the diagram template
     * @return Command line handler state
     */
    cli_flow_t add_config_diagram_from_template(
        const std::string &config_file_path, const std::string &template_name);

    /**
     * Check if diagram output directory exists, if not create it
     *
     * @param dir Path to the output directory
     * @return True if directory exists or has been created
     */
    bool ensure_output_directory_exists(const std::string &dir);

    /**
     * @brief Combines runtime configuration parameters into a single structure
     *
     * @return Runtime config instance
     */
    runtime_config get_runtime_config() const;

    /**
     * @brief Set the default config path
     *
     * @param path
     */
    void set_config_path(const std::string &path);

    std::string config_path{".clang-uml"};
    std::optional<std::string> compilation_database_dir{};
    std::vector<std::string> diagram_names{};
    std::optional<std::string> output_directory{};
    std::string effective_output_directory{};
    unsigned int thread_count{};
    bool show_version{false};
    int verbose{};
    bool progress{false};
    bool list_diagrams{false};
    bool quiet{false};
    bool initialize{false};
    bool allow_empty_diagrams{false};
    std::optional<std::vector<std::string>> add_compile_flag;
    std::optional<std::vector<std::string>> remove_compile_flag;
#if !defined(_WIN32)
    std::optional<std::string> query_driver;
#endif
    std::optional<std::string> add_class_diagram;
    std::optional<std::string> add_sequence_diagram;
    std::optional<std::string> add_package_diagram;
    std::optional<std::string> add_include_diagram;
    std::optional<std::string> add_diagram_from_template;
    bool dump_config{false};
    bool print_from{false};
    bool print_to{false};
    std::optional<bool> paths_relative_to_pwd{};
    std::vector<std::string> template_variables{};
    bool list_templates{false};
    std::optional<bool> no_metadata{};
    std::optional<std::string> show_template;
    std::vector<clanguml::common::generator_type_t> generators{
        clanguml::common::generator_type_t::plantuml};
    bool no_validate{false};
    bool validate_only{false};
    bool render_diagrams{false};
    std::optional<std::string> plantuml_cmd;
    std::optional<std::string> mermaid_cmd;

    clanguml::config::config config;

private:
    /**
     * This method parses the command line options using CLI11 library.
     *
     * @param argc
     * @param argv
     * @return Command line handler state
     */
    cli_flow_t parse(int argc, const char **argv);

    /**
     * Handle command line options before parsing the configuration file
     *
     * @return Command line handler state
     */
    cli_flow_t handle_pre_config_options();

    /**
     * Load configuration file from file or stdin
     *
     * @return Command line handler state
     */
    cli_flow_t load_config();

    /**
     * Handle command line options before parsing the configuration file
     *
     * @return Command line handler state
     */
    cli_flow_t handle_post_config_options();

    /**
     * Setup spdlog library depending on provided command line options
     */
    void setup_logging();

    std::ostream &ostr_;
    std::shared_ptr<spdlog::logger> logger_;
    CLI::App app{"Clang-based UML diagram generator for C++"};
};
} // namespace clanguml::cli