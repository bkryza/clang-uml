/**
 * src/main.cc
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

#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "config/config.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "util/util.h"
#include "version.h"

#ifndef NDEBUG
#include <backward-cpp/backward.hpp>
#endif

#include <clang/Basic/Version.h>
#include <clang/Config/config.h>
#include <cli11/CLI11.hpp>
#include <spdlog/spdlog.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <util/thread_pool_executor.h>

#ifndef NDEBUG
namespace backward {
backward::SignalHandling sh; // NOLINT
} // namespace backward
#endif

using namespace clanguml;
using config::config;

/**
 * Print the program version and basic information
 */
void print_version();

/**
 * Print list of diagrams available in the configuration file
 *
 *  @param cfg Configuration instance loaded from configuration file
 */
void print_diagrams_list(const clanguml::config::config &cfg);

/**
 * Generate sample configuration file and exit.
 *
 * @return 0 on success or error code
 */
int create_config_file();

/**
 * Add example diagram of given type to the config file.
 *
 * @param type Type of the sample diagram to add
 * @param config_file_path Path to the config file
 * @param name Name of the new diagram
 * @return 0 on success or error code
 */
int add_config_diagram(clanguml::common::model::diagram_t type,
    const std::string &config_file_path, const std::string &name);

/**
 * Check if diagram output directory exists, if not create it
 *
 * @param dir Path to the output directory
 * @return True if directory exists or has been created
 */
bool ensure_output_directory_exists(const std::string &dir);

/**
 * Generate specific diagram identified by name
 *
 * @param od Diagram output directory
 * @param name Name of the diagram as specified in the config file
 * @param diagram Diagram model instance
 * @param db Compilation database
 * @param translation_units List of translation units to be used for this
 *        diagram
 * @param verbose Logging level
 */
void generate_diagram(const std::string &od, const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const clang::tooling::CompilationDatabase &db,
    const std::vector<std::string> &translation_units, bool verbose);

/**
 * Find translation units for diagrams.
 *
 *  For each diagram to be generated, this function selects translation units
 *  to be used for this diagram. The files are selected as an intersection
 *  between all translation units found in the compilation database and the
 *  `glob` patterns specified for each diagram in the configuration file.
 *
 *  @param diagram_names List of diagram names to be generated
 *  @param config Configuration instance
 *  @param compilation_database_files All translation units in compilation
 *  database
 *  @param translation_units_map The output map containing translation
 *                  units for each diagram by name
 */
void find_translation_units_for_diagrams(
    const std::vector<std::string> &diagram_names,
    clanguml::config::config &config,
    const std::vector<std::string> &compilation_database_files,
    std::map<std::string, std::vector<std::string>> &translation_units_map);

/**
 *  Generate diagrams.
 *
 *  This function generates all diagrams specified in the configuration file
 *  and in the command line.
 *
 *  @param diagram_names List of diagram names to be generated
 *  @param config Configuration instance
 *  @param od Output directory where diagrams should be written
 *  @param db Compilation database instance
 *  @param verbose Verbosity level
 *  @param thread_count Number of diagrams to be generated in parallel
 *  @param translation_units_map List of translation units to be used for each
 *         diagram
 */
void generate_diagrams(const std::vector<std::string> &diagram_names,
    clanguml::config::config &config, const std::string &od,
    const std::unique_ptr<clang::tooling::CompilationDatabase> &db, int verbose,
    unsigned int thread_count,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map);

int main(int argc, const char *argv[])
{
    CLI::App app{"Clang-based PlantUML diagram generator for C++"};

    std::string config_path{".clang-uml"};
    std::string compilation_database_dir{'.'};
    std::vector<std::string> diagram_names{};
    std::optional<std::string> output_directory;
    unsigned int thread_count{0};
    bool show_version{false};
    int verbose{0};
    bool list_diagrams{false};
    bool quiet{false};
    bool initialize{false};
    std::optional<std::string> add_class_diagram;
    std::optional<std::string> add_sequence_diagram;
    std::optional<std::string> add_package_diagram;
    std::optional<std::string> add_include_diagram;

    app.add_option(
        "-c,--config", config_path, "Location of configuration file");
    app.add_option("-d,--compile-database", compilation_database_dir,
        "Location of compilation database directory");
    app.add_option("-n,--diagram-name", diagram_names,
        "List of diagram names to generate");
    app.add_option("-o,--output-directory", output_directory,
        "Override output directory specified in config file");
    app.add_option("-t,--thread-count", thread_count,
        "Thread pool size (0 = hardware concurrency)");
    app.add_flag("-V,--version", show_version, "Print version and exit");
    app.add_flag("-v,--verbose", verbose,
        "Verbose logging (use multiple times to increase - e.g. -vvv)");
    app.add_flag("-q,--quiet", quiet, "Minimal logging");
    app.add_flag("-l,--list-diagrams", list_diagrams,
        "Print list of diagrams defined in the config file");
    app.add_flag("--init", initialize, "Initialize example config file");
    app.add_option(
        "--add-class-diagram", add_class_diagram, "Add class diagram config");
    app.add_option("--add-sequence-diagram", add_sequence_diagram,
        "Add sequence diagram config");
    app.add_option("--add-package-diagram", add_package_diagram,
        "Add package diagram config");
    app.add_option("--add-include-diagram", add_include_diagram,
        "Add include diagram config");

    CLI11_PARSE(app, argc, argv);

    if (show_version) {
        print_version();
        return 0;
    }

    if (initialize) {
        return create_config_file();
    }

    verbose++;

    if (quiet)
        verbose = 0;

    if (add_class_diagram) {
        return add_config_diagram(clanguml::common::model::diagram_t::kClass,
            config_path, *add_class_diagram);
    }

    if (add_sequence_diagram) {
        return add_config_diagram(clanguml::common::model::diagram_t::kSequence,
            config_path, *add_sequence_diagram);
    }

    if (add_package_diagram) {
        return add_config_diagram(clanguml::common::model::diagram_t::kPackage,
            config_path, *add_package_diagram);
    }

    if (add_include_diagram) {
        return add_config_diagram(clanguml::common::model::diagram_t::kInclude,
            config_path, *add_include_diagram);
    }

    clanguml::util::setup_logging(verbose);

    clanguml::config::config config;
    try {
        config = clanguml::config::load(config_path);
    }
    catch (std::runtime_error &e) {
        LOG_ERROR(e.what());
        return 1;
    }

    if (list_diagrams) {
        print_diagrams_list(config);
        return 0;
    }

    LOG_INFO("Loaded clang-uml config from {}", config_path);

    LOG_INFO("Loading compilation database from {} directory",
        config.compilation_database_dir());

    auto od = config.output_directory();
    if (output_directory)
        od = output_directory.value();

    if (!ensure_output_directory_exists(od))
        return 1;

    std::string err{};
    auto db = clang::tooling::CompilationDatabase::autoDetectFromDirectory(
        config.compilation_database_dir(), err);

    if (!err.empty()) {
        LOG_ERROR("Failed to load compilation database from {}",
            config.compilation_database_dir());
        return 1;
    }

    const auto compilation_database_files = db->getAllFiles();

    std::map<std::string /* diagram name */,
        std::vector<std::string> /*translation units*/>
        translation_units_map;

    // We have to generate the translation units list for each diagram before
    // scheduling tasks, because std::filesystem::current_path cannot be trusted
    // with multiple threads
    find_translation_units_for_diagrams(diagram_names, config,
        compilation_database_files, translation_units_map);

    generate_diagrams(diagram_names, config, od, db, verbose, thread_count,
        translation_units_map);

    return 0;
}

void generate_diagram(const std::string &od, const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const clang::tooling::CompilationDatabase &db,
    const std::vector<std::string> &translation_units, bool verbose)
{
    using clanguml::common::model::diagram_t;
    using clanguml::config::class_diagram;
    using clanguml::config::include_diagram;
    using clanguml::config::package_diagram;
    using clanguml::config::sequence_diagram;

    auto path = std::filesystem::path{od} / fmt::format("{}.puml", name);
    std::ofstream ofs;
    ofs.open(path, std::ofstream::out | std::ofstream::trunc);

    if (diagram->type() == diagram_t::kClass) {
        using diagram_config = class_diagram;
        using diagram_model = clanguml::class_diagram::model::diagram;
        using diagram_visitor =
            clanguml::class_diagram::visitor::translation_unit_visitor;

        auto model =
            clanguml::common::generators::plantuml::generate<diagram_model,
                diagram_config, diagram_visitor>(db, diagram->name,
                dynamic_cast<diagram_config &>(*diagram), translation_units,
                verbose);

        ofs << clanguml::class_diagram::generators::plantuml::generator(
            dynamic_cast<diagram_config &>(*diagram), *model);
    }
    else if (diagram->type() == diagram_t::kSequence) {
        using diagram_config = sequence_diagram;
        using diagram_model = clanguml::sequence_diagram::model::diagram;
        using diagram_visitor =
            clanguml::sequence_diagram::visitor::translation_unit_visitor;

        auto model =
            clanguml::common::generators::plantuml::generate<diagram_model,
                diagram_config, diagram_visitor>(db, diagram->name,
                dynamic_cast<diagram_config &>(*diagram), translation_units,
                verbose);

        ofs << clanguml::sequence_diagram::generators::plantuml::generator(
            dynamic_cast<clanguml::config::sequence_diagram &>(*diagram),
            *model);
    }
    else if (diagram->type() == diagram_t::kPackage) {
        using diagram_config = package_diagram;
        using diagram_model = clanguml::package_diagram::model::diagram;
        using diagram_visitor =
            clanguml::package_diagram::visitor::translation_unit_visitor;

        auto model =
            clanguml::common::generators::plantuml::generate<diagram_model,
                diagram_config, diagram_visitor>(db, diagram->name,
                dynamic_cast<diagram_config &>(*diagram), translation_units,
                verbose);

        ofs << clanguml::package_diagram::generators::plantuml::generator(
            dynamic_cast<diagram_config &>(*diagram), *model);
    }
    else if (diagram->type() == diagram_t::kInclude) {
        using diagram_config = include_diagram;
        using diagram_model = clanguml::include_diagram::model::diagram;
        using diagram_visitor =
            clanguml::include_diagram::visitor::translation_unit_visitor;

        auto model =
            clanguml::common::generators::plantuml::generate<diagram_model,
                diagram_config, diagram_visitor>(db, diagram->name,
                dynamic_cast<diagram_config &>(*diagram), translation_units,
                verbose);

        ofs << clanguml::include_diagram::generators::plantuml::generator(
            dynamic_cast<diagram_config &>(*diagram), *model);
    }

    LOG_INFO("Written {} diagram to {}", name, path.string());

    ofs.close();
}

void generate_diagrams(const std::vector<std::string> &diagram_names,
    clanguml::config::config &config, const std::string &od,
    const std::unique_ptr<clang::tooling::CompilationDatabase> &db,
    const int verbose, const unsigned int thread_count,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map)
{
    util::thread_pool_executor generator_executor{thread_count};
    std::vector<std::future<void>> futs;

    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command line,
        // and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        const auto &valid_translation_units = translation_units_map.at(name);

        if (valid_translation_units.empty()) {
            LOG_ERROR(
                "Diagram {} generation failed: no translation units found",
                name);
            continue;
        }

        futs.emplace_back(generator_executor.add(
            [&od, &name = name, &diagram = diagram, db = std::ref(*db),
                translation_units = valid_translation_units, verbose]() {
                try {
                    generate_diagram(
                        od, name, diagram, db, translation_units, verbose != 0);
                }
                catch (std::runtime_error &e) {
                    LOG_ERROR(e.what());
                }
            }));
    }

    for (auto &fut : futs) {
        fut.get();
    }
}

void find_translation_units_for_diagrams(
    const std::vector<std::string> &diagram_names,
    clanguml::config::config &config,
    const std::vector<std::string> &compilation_database_files,
    std::map<std::string, std::vector<std::string>> &translation_units_map)
{
    const auto current_directory = std::filesystem::current_path();

    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command line,
        // and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        // If glob is not defined use all translation units from the
        // compilation database
        if (!diagram->glob.has_value) {
            translation_units_map[name] = compilation_database_files;
        }
        // Otherwise, get all translation units matching the glob from diagram
        // configuration
        else {
            const std::vector<std::string> translation_units =
                diagram->get_translation_units(current_directory);

            std::vector<std::string> valid_translation_units{};
            std::copy_if(compilation_database_files.begin(),
                compilation_database_files.end(),
                std::back_inserter(valid_translation_units),
                [&translation_units](const auto &tu) {
                    return util::contains(translation_units, tu);
                });

            translation_units_map[name] = std::move(valid_translation_units);
        }
    }
}

bool ensure_output_directory_exists(const std::string &dir)
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

void print_version()
{
    constexpr auto kLLVMBackendPackageStringLength{5};
    std::cout << "clang-uml " << clanguml::version::CLANG_UML_VERSION << '\n';
    std::cout << "Copyright (C) 2021-2022 Bartek Kryza <bkryza@gmail.com>"
              << '\n';
    std::cout << "Built against LLVM/Clang libraries version: "
              << std::string{BACKEND_PACKAGE_STRING}.substr(
                     kLLVMBackendPackageStringLength)
              << std::endl;
    std::cout << "Using LLVM/Clang libraries version: "
              << clang::getClangFullVersion() << std::endl;
}

void print_diagrams_list(const clanguml::config::config &cfg)
{
    using std::cout;

    cout << "The following diagrams are defined in the config file:\n";
    for (const auto &[name, diagram] : cfg.diagrams) {
        cout << "  - " << name << " [" << to_string(diagram->type()) << "]";
        cout << '\n';
    }
}

int create_config_file()
{
    namespace fs = std::filesystem;
    using std::cout;

    fs::path config_file{"./.clang-uml"};

    if (fs::exists(config_file)) {
        cout << "ERROR: .clang-uml file already exists\n";
        return 1;
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

    return 0;
}

int add_config_diagram(clanguml::common::model::diagram_t type,
    const std::string &config_file_path, const std::string &name)
{
    fs::path config_file{config_file_path};

    if (!fs::exists(config_file)) {
        std::cerr << "ERROR: " << config_file_path << " file doesn't exists\n";
        return 1;
    }

    YAML::Node doc = YAML::LoadFile(config_file);

    for (YAML::const_iterator it = doc["diagrams"].begin();
         it != doc["diagrams"].end(); ++it) {
        if (it->first.as<std::string>() == name) {
            std::cerr << "ERROR: " << config_file_path
                      << " file already contains '" << name << "' diagram";
            return 1;
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

    return 0;
}