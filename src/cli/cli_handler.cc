/**
 * @file src/options/cli_handler.cc
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
#include "cli_handler.h"

#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "util/util.h"
#include "version.h"

#include <clang/Basic/Version.h>
#include <clang/Config/config.h>
#include <indicators/indicators.hpp>

namespace clanguml::cli {
cli_handler::cli_handler(
    std::ostream &ostr, std::shared_ptr<spdlog::logger> logger)
    : ostr_{ostr}
    , logger_{std::move(logger)}
{
}

void cli_handler::setup_logging()
{
    spdlog::drop("clanguml-logger");

    spdlog::register_logger(logger_);

    logger_->set_pattern("[%^%l%^] [tid %t] %v");

    if (verbose == 0) {
        logger_->set_level(spdlog::level::err);
    }
    else if (verbose == 1) {
        logger_->set_level(spdlog::level::info);
    }
    else if (verbose == 2) {
        logger_->set_level(spdlog::level::debug);
    }
    else {
        logger_->set_level(spdlog::level::trace);
    }
}

cli_flow_t cli_handler::parse(int argc, const char **argv)
{
    static const std::map<std::string, clanguml::common::generator_type_t>
        generator_type_names{
            {"plantuml", clanguml::common::generator_type_t::plantuml},
            {"json", clanguml::common::generator_type_t::json},
            {"mermaid", clanguml::common::generator_type_t::mermaid}};

    app.add_option("-c,--config", config_path,
        "Location of configuration file, when '-' read from stdin");
    app.add_option("-d,--compile-database", compilation_database_dir,
        "Location of compilation database directory");
    app.add_option("-n,--diagram-name", diagram_names,
        "List of diagram names to generate");
    app.add_option("-g,--generator", generators,
           "Name of the generator (default: plantuml)")
        ->transform(CLI::CheckedTransformer(generator_type_names));
    app.add_option("-o,--output-directory", output_directory,
        "Override output directory specified in config file");
    app.add_option("-t,--thread-count", thread_count,
        "Thread pool size (0 = hardware concurrency)");
    app.add_flag("-V,--version", show_version, "Print version and exit");
    app.add_flag("-v,--verbose", verbose,
        "Verbose logging (use multiple times to increase - e.g. -vvv)");
    app.add_flag(
        "-p,--progress", progress, "Show progress bars for generated diagrams");
    app.add_flag("-q,--quiet", quiet, "Minimal logging");
    app.add_flag("-l,--list-diagrams", list_diagrams,
        "Print list of diagrams defined in the config file");
    app.add_flag("--init", initialize, "Initialize example config file");
    app.add_option("--add-compile-flag", add_compile_flag,
        "Add a compilation flag to each entry in the compilation database");
    app.add_option("--remove-compile-flag", remove_compile_flag,
        "Remove a compilation flag from each entry in the compilation "
        "database");
#if !defined(_WIN32)
    app.add_option("--query-driver", query_driver,
        "Query the specific compiler driver to extract system paths and add to "
        "compile commands (e.g. arm-none-eabi-g++)");
#endif
    app.add_flag("--allow-empty-diagrams", allow_empty_diagrams,
        "Do not raise an error when generated diagram model is empty");
    app.add_option(
        "--add-class-diagram", add_class_diagram, "Add class diagram config");
    app.add_option("--add-sequence-diagram", add_sequence_diagram,
        "Add sequence diagram config");
    app.add_option("--add-package-diagram", add_package_diagram,
        "Add package diagram config");
    app.add_option("--add-include-diagram", add_include_diagram,
        "Add include diagram config");
    app.add_option("--add-diagram-from-template", add_diagram_from_template,
        "Add diagram config based on diagram template");
    app.add_option("--template-var", template_variables,
        "Specify a value for a template variable");
    app.add_flag("--list-templates", list_templates,
        "List all available diagram templates");
    app.add_option("--show-template", show_template,
        "Show specific diagram template definition");
    app.add_flag(
        "--dump-config", dump_config, "Print effective config to stdout");
    app.add_flag("--paths-relative-to-pwd", paths_relative_to_pwd,
        "If true, all paths in configuration files are relative to the $PWD "
        "instead of actual location of `.clang-uml` file.");
    app.add_flag("--no-metadata", no_metadata,
        "Skip metadata (e.g. clang-uml version) from diagrams");
    app.add_flag("--print-from,--print-start-from", print_from,
        "Print all possible 'from' values for a given diagram");
    app.add_flag("--print-to", print_to,
        "Print all possible 'to' values for a given diagram");
    app.add_flag("--no-validate", no_validate,
        "Do not perform configuration file schema validation");
    app.add_flag("--validate-only", validate_only,
        "Perform configuration file schema validation and exit");
    app.add_flag("-r,--render_diagrams", render_diagrams,
        "Automatically render generated diagrams using appropriate command");
    app.add_option("--plantuml-cmd", plantuml_cmd,
        "Command template to render PlantUML diagram, `{}` will be replaced "
        "with diagram name.");
    app.add_option("--mermaid-cmd", mermaid_cmd,
        "Command template to render MermaidJS diagram, `{}` will be replaced "
        "with diagram name.");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::CallForHelp &e) {
        exit(app.exit(e)); // NOLINT(concurrency-mt-unsafe)
    }
    catch (const CLI::Success &e) {
        return cli_flow_t::kExit;
    }
    catch (const CLI::ParseError &e) {
        exit(app.exit(e)); // NOLINT(concurrency-mt-unsafe)
    }

    if (quiet || dump_config || print_from || print_to)
        verbose = 0;
    else
        verbose++;

    if (progress)
        verbose = 0;

    return cli_flow_t::kContinue;
}

cli_flow_t cli_handler::handle_options(int argc, const char **argv)
{
    auto res = parse(argc, argv);

    if (res != cli_flow_t::kContinue)
        return res;

    setup_logging();

    res = handle_pre_config_options();

    if (res != cli_flow_t::kContinue)
        return res;

    res = load_config();

    if (res != cli_flow_t::kContinue)
        return res;

    res = handle_post_config_options();

    config.inherit();

    if (progress) {
        spdlog::drop("clanguml-logger");

        // Setup null logger for clean progress indicators
        std::vector<spdlog::sink_ptr> sinks;
        logger_ = std::make_shared<spdlog::logger>(
            "clanguml-logger", begin(sinks), end(sinks));
        spdlog::register_logger(logger_);
    }

    return res;
}

cli_flow_t cli_handler::handle_pre_config_options()
{
    if (show_version) {
        return print_version();
    }

    if ((config_path == "-") &&
        (initialize || add_diagram_from_template ||
            add_class_diagram.has_value() || add_sequence_diagram.has_value() ||
            add_package_diagram.has_value() ||
            add_include_diagram.has_value())) {

        LOG_ERROR(
            "ERROR: Cannot add a diagram config to configuration from stdin");

        return cli_flow_t::kError;
    }

    if (print_from || print_to) {
        if (diagram_names.size() != 1) {
            LOG_ERROR("ERROR: '--print-from' and '--print-to' require "
                      "specifying one diagram name using '-n' option");

            return cli_flow_t::kError;
        }
    }

    if (initialize) {
        return create_config_file();
    }

    if (config_path != "-") {
        if (add_class_diagram) {
            return add_config_diagram(
                clanguml::common::model::diagram_t::kClass, config_path,
                *add_class_diagram);
        }

        if (add_sequence_diagram) {
            return add_config_diagram(
                clanguml::common::model::diagram_t::kSequence, config_path,
                *add_sequence_diagram);
        }

        if (add_package_diagram) {
            return add_config_diagram(
                clanguml::common::model::diagram_t::kPackage, config_path,
                *add_package_diagram);
        }

        if (add_include_diagram) {
            return add_config_diagram(
                clanguml::common::model::diagram_t::kInclude, config_path,
                *add_include_diagram);
        }
    }

    return cli_flow_t::kContinue;
}

cli_flow_t cli_handler::load_config()
{
    try {
        config = clanguml::config::load(config_path, false,
            paths_relative_to_pwd, no_metadata, !no_validate);
        if (validate_only) {
            std::cout << "Configuration file " << config_path << " is valid.\n";

            return cli_flow_t::kExit;
        }

        return cli_flow_t::kContinue;
    }
    catch (std::runtime_error &e) {
        LOG_ERROR(e.what());
    }

    return cli_flow_t::kError;
}

cli_flow_t cli_handler::handle_post_config_options()
{
    if (dump_config) {
        return print_config();
    }

    if (list_diagrams) {
        return print_diagrams_list();
    }

    if (list_templates) {
        return print_diagram_templates();
    }

    if (show_template) {
        return print_diagram_template(show_template.value());
    }

    if (config_path != "-" && add_diagram_from_template) {
        return add_config_diagram_from_template(
            config_path, add_diagram_from_template.value());
    }

    LOG_INFO("Loaded clang-uml config from {}", config_path);

    if (allow_empty_diagrams) {
        config.allow_empty_diagrams.set(true);
    }

    //
    // Override selected config options from command line
    //
    if (compilation_database_dir) {
        config.compilation_database_dir.set(
            util::ensure_path_is_absolute(compilation_database_dir.value())
                .string());
    }

    effective_output_directory = config.output_directory();

    // Override the output directory from the config
    // with the value from the command line if any
    if (output_directory)
        effective_output_directory = output_directory.value();

    if (output_directory) {
        config.output_directory.set(
            util::ensure_path_is_absolute(output_directory.value()).string());
    }

    LOG_INFO("Loading compilation database from {} directory",
        config.compilation_database_dir());

    if (!ensure_output_directory_exists(effective_output_directory))
        return cli_flow_t::kError;

    //
    // Append add_compile_flags and remove_compile_flags to the config
    //
    if (add_compile_flag) {
        std::copy(add_compile_flag->begin(), add_compile_flag->end(),
            std::back_inserter(config.add_compile_flags.value));
        config.add_compile_flags.has_value = true;
    }

    if (remove_compile_flag) {
        std::copy(remove_compile_flag->begin(), remove_compile_flag->end(),
            std::back_inserter(config.remove_compile_flags.value));
        config.remove_compile_flags.has_value = true;
    }

    if (plantuml_cmd) {
        if (!config.puml)
            config.puml.set({});

        config.puml().cmd = plantuml_cmd.value();
    }

    if (mermaid_cmd) {
        if (!config.mermaid)
            config.mermaid.set({});

        config.mermaid().cmd = mermaid_cmd.value();
    }

#if !defined(_WIN32)
    if (query_driver) {
        config.query_driver.set(*query_driver);
    }
#endif

    return cli_flow_t::kContinue;
}

runtime_config cli_handler::get_runtime_config() const
{
    runtime_config cfg;
    cfg.generators = generators;
    cfg.verbose = verbose;
    cfg.print_from = print_from;
    cfg.print_to = print_to;
    cfg.progress = progress;
    cfg.thread_count = thread_count;
    cfg.render_diagrams = render_diagrams;
    cfg.output_directory = effective_output_directory;

    return cfg;
}

void cli_handler::set_config_path(const std::string &path)
{
    config_path = path;
}

cli_flow_t cli_handler::print_version()
{
    ostr_ << "clang-uml " << clanguml::version::CLANG_UML_VERSION << '\n';
    ostr_ << "Copyright (C) 2021-2024 Bartek Kryza <bkryza@gmail.com>" << '\n';
    ostr_ << util::get_os_name() << '\n';
    ostr_ << "Built against LLVM/Clang libraries version: "
          << LLVM_VERSION_STRING << '\n';
    ostr_ << "Using LLVM/Clang libraries version: "
          << clang::getClangFullVersion() << '\n';

    return cli_flow_t::kExit;
}

bool cli_handler::ensure_output_directory_exists(const std::string &dir)
{
    namespace fs = std::filesystem;
    using std::cout;

    fs::path output_dir{dir};

    if (fs::exists(output_dir) && !fs::is_directory(output_dir)) {
        cout << "ERROR: " << dir << " is not a directory...\n";
        return false;
    }

    if (!fs::exists(output_dir)) {
        return fs::create_directories(output_dir);
    }

    return true;
}

cli_flow_t cli_handler::print_diagrams_list()
{
    using std::cout;

    ostr_ << "The following diagrams are defined in the config file:\n";
    for (const auto &[name, diagram] : config.diagrams) {
        ostr_ << "  - " << name << " [" << to_string(diagram->type()) << "]";
        ostr_ << '\n';
    }

    return cli_flow_t::kExit;
}

cli_flow_t cli_handler::print_diagram_templates()
{
    using std::cout;

    if (!config.diagram_templates) {
        ostr_ << "No diagram templates are defined in the config file\n";
        return cli_flow_t::kExit;
    }

    ostr_ << "The following diagram templates are available:\n";
    for (const auto &[name, diagram_template] : config.diagram_templates()) {
        ostr_ << "  - " << name << " [" << to_string(diagram_template.type)
              << "]";
        if (!diagram_template.description.empty())
            ostr_ << ": " << diagram_template.description;
        ostr_ << '\n';
    }

    return cli_flow_t::kExit;
}

cli_flow_t cli_handler::print_diagram_template(const std::string &template_name)
{
    if (!config.diagram_templates ||
        config.diagram_templates().count(template_name) == 0) {
        ostr_ << "No such diagram template: " << template_name << "\n";
        return cli_flow_t::kError;
    }

    for (const auto &[name, diagram_template] : config.diagram_templates()) {
        if (template_name == name) {
            ostr_ << diagram_template.jinja_template << "\n";
            return cli_flow_t::kExit;
        }
    }

    return cli_flow_t::kError;
}

cli_flow_t cli_handler::create_config_file()
{
    namespace fs = std::filesystem;

    fs::path config_file{config_path};

    if (fs::exists(config_file)) {
        ostr_ << "ERROR: .clang-uml file already exists\n";
        return cli_flow_t::kError;
    }

    YAML::Emitter out;
    out.SetIndent(2);
    out << YAML::BeginMap;
    out << YAML::Comment("Change to directory where compile_commands.json is");
    out << YAML::Key << "compilation_database_dir" << YAML::Value << ".";
    out << YAML::Newline
        << YAML::Comment("Change to directory where diagram should be written");
    out << YAML::Key << "output_directory" << YAML::Value << "docs/diagrams";
    out << YAML::Key << "diagrams" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "example_class_diagram" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << "class";
    out << YAML::Key << "glob" << YAML::Value;
    out << YAML::BeginSeq << "src/*.cpp" << YAML::EndSeq;
    out << YAML::Key << "using_namespace" << YAML::Value;
    out << YAML::BeginSeq << "myproject" << YAML::EndSeq;
    out << YAML::Key << "include";
    out << YAML::BeginMap;
    out << YAML::Key << "namespaces";
    out << YAML::BeginSeq << "myproject" << YAML::EndSeq;
    out << YAML::EndMap;
    out << YAML::Key << "exclude";
    out << YAML::BeginMap;
    out << YAML::Key << "namespaces";
    out << YAML::BeginSeq << "myproject::detail" << YAML::EndSeq;
    out << YAML::EndMap;
    out << YAML::EndMap;
    out << YAML::EndMap;
    out << YAML::EndMap;
    out << YAML::Newline;

    std::ofstream ofs(config_file);
    ofs << out.c_str();
    ofs.close();

    return cli_flow_t::kExit;
}

cli_flow_t cli_handler::add_config_diagram(
    clanguml::common::model::diagram_t type,
    const std::string &config_file_path, const std::string &name)
{
    namespace fs = std::filesystem;

    fs::path config_file{config_file_path};

    if (!fs::exists(config_file)) {
        std::cerr << "ERROR: " << config_file_path << " file doesn't exists\n";
        return cli_flow_t::kError;
    }

    YAML::Node doc = YAML::LoadFile(config_file.string());

    for (YAML::const_iterator it = doc["diagrams"].begin();
         it != doc["diagrams"].end(); ++it) {
        if (it->first.as<std::string>() == name) {
            std::cerr << "ERROR: " << config_file_path
                      << " file already contains '" << name << "' diagram";
            return cli_flow_t::kError;
        }
    }

    if (type == clanguml::common::model::diagram_t::kClass) {
        doc["diagrams"][name]["type"] = "class";
        doc["diagrams"][name]["glob"] = std::vector<std::string>{{"src/*.cpp"}};
        doc["diagrams"][name]["using_namespace"] =
            std::vector<std::string>{{"myproject"}};
        doc["diagrams"][name]["include"]["namespaces"] =
            std::vector<std::string>{{"myproject"}};
        doc["diagrams"][name]["exclude"]["namespaces"] =
            std::vector<std::string>{{"myproject::detail"}};
    }
    else if (type == clanguml::common::model::diagram_t::kSequence) {
        doc["diagrams"][name]["type"] = "sequence";
        doc["diagrams"][name]["glob"] = std::vector<std::string>{{"src/*.cpp"}};
        doc["diagrams"][name]["combine_free_functions_into_file_participants"] =
            true;
        doc["diagrams"][name]["inline_lambda_messages"] = false;
        doc["diagrams"][name]["generate_message_comments"] = false;
        doc["diagrams"][name]["generate_condition_statements"] = false;
        doc["diagrams"][name]["using_namespace"] =
            std::vector<std::string>{{"myproject"}};
        doc["diagrams"][name]["include"]["paths"] =
            std::vector<std::string>{{"src"}};
        doc["diagrams"][name]["exclude"]["namespaces"] =
            std::vector<std::string>{{"myproject::detail"}};
        doc["diagrams"][name]["start_from"] =
            std::vector<std::map<std::string, std::string>>{
                {{"function", "main(int,const char **)"}}};
    }
    else if (type == clanguml::common::model::diagram_t::kPackage) {
        doc["diagrams"][name]["type"] = "package";
        doc["diagrams"][name]["glob"] = std::vector<std::string>{{"src/*.cpp"}};
        doc["diagrams"][name]["using_namespace"] =
            std::vector<std::string>{{"myproject"}};
        doc["diagrams"][name]["include"]["namespaces"] =
            std::vector<std::string>{{"myproject"}};
        doc["diagrams"][name]["exclude"]["namespaces"] =
            std::vector<std::string>{{"myproject::detail"}};
    }
    else if (type == clanguml::common::model::diagram_t::kInclude) {
        doc["diagrams"][name]["type"] = "include";
        doc["diagrams"][name]["glob"] = std::vector<std::string>{{"src/*.cpp"}};
        doc["diagrams"][name]["relative_to"] = ".";
        doc["diagrams"][name]["include"]["paths"] =
            std::vector<std::string>{{"src"}};
    }

    YAML::Emitter out;
    out.SetIndent(2);

    out << doc;
    out << YAML::Newline;

    std::ofstream ofs(config_file);
    ofs << out.c_str();
    ofs.close();

    return cli_flow_t::kExit;
}

cli_flow_t cli_handler::add_config_diagram_from_template(
    const std::string &config_file_path, const std::string &template_name)
{
    if (!config.diagram_templates ||
        !(config.diagram_templates().find(template_name) !=
            config.diagram_templates().end())) {
        std::cerr << "ERROR: No such diagram template: " << template_name
                  << "\n";
        return cli_flow_t::kError;
    }

    // First, try to render the template using inja and create a YAML node
    // from it
    inja::json ctx;
    for (const auto &tv : template_variables) {
        const auto var = util::split(tv, "=");
        if (var.size() != 2) {
            std::cerr << "ERROR: Invalid template variable " << tv << "\n";
            return cli_flow_t::kError;
        }

        ctx[var.at(0)] = var.at(1);
    }

    auto diagram_template_str =
        config.diagram_templates().at(template_name).jinja_template;

    YAML::Node diagram_node;

    try {
        auto diagram_str = inja::render(diagram_template_str, ctx);
        diagram_node = YAML::Load(diagram_str);
    }
    catch (inja::InjaError &e) {
        std::cerr << "ERROR: Failed to generate diagram template '"
                  << template_name << "': " << e.what() << "\n";
        return cli_flow_t::kError;
    }
    catch (YAML::Exception &e) {
        std::cerr << "ERROR: Rendering diagram template '" << template_name
                  << "' resulted in invalid YAML: " << e.what() << "\n";
        return cli_flow_t::kError;
    }

    namespace fs = std::filesystem;

    fs::path config_file{config_file_path};

    if (!fs::exists(config_file)) {
        std::cerr << "ERROR: " << config_file_path << " file doesn't exists\n";
        return cli_flow_t::kError;
    }

    YAML::Node doc = YAML::LoadFile(config_file.string());

    const auto diagram_name = diagram_node.begin()->first.as<std::string>();
    doc["diagrams"][diagram_name] = diagram_node.begin()->second;

    YAML::Emitter out;
    out.SetIndent(2);

    out << doc;
    out << YAML::Newline;

    std::ofstream ofs(config_file);
    ofs << out.c_str();
    ofs.close();

    return cli_flow_t::kExit;
}

cli_flow_t cli_handler::print_config()
{
    YAML::Emitter out;
    out.SetIndent(2);

    out << config;
    out << YAML::Newline;

    ostr_ << out.c_str();

    return cli_flow_t::kExit;
}
} // namespace clanguml::cli