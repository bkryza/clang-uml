/**
 * src/common/generators/generators.cc
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

#include "generators.h"

namespace clanguml::common::generators {
void find_translation_units_for_diagrams(
    const std::vector<std::string> &diagram_names,
    clanguml::config::config &config,
    const std::vector<std::string> &compilation_database_files,
    std::map<std::string, std::vector<std::string>> &translation_units_map)
{
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
                diagram->get_translation_units();

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

namespace detail {

template <typename DiagramConfig, typename GeneratorTag, typename DiagramModel>
void generate_diagram_select_generator(const std::string &od,
    const std::string &name, std::shared_ptr<clanguml::config::diagram> diagram,
    const DiagramModel &model)
{
    using diagram_generator =
        typename diagram_generator_t<DiagramConfig, GeneratorTag>::type;

    auto path = std::filesystem::path{od} /
        fmt::format("{}.{}", name, GeneratorTag::extension);
    std::ofstream ofs;
    ofs.open(path, std::ofstream::out | std::ofstream::trunc);
    ofs << diagram_generator(dynamic_cast<DiagramConfig &>(*diagram), *model);

    ofs.close();

    LOG_INFO("Written {} diagram to {}", name, path.string());
}

template <typename DiagramConfig>
void generate_diagram_impl(const std::string &od, const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const clang::tooling::CompilationDatabase &db,
    const std::vector<std::string> &translation_units,
    const std::vector<clanguml::common::generator_type_t> &generators,
    bool verbose)
{
    using diagram_config = DiagramConfig;
    using diagram_model = typename diagram_model_t<DiagramConfig>::type;
    using diagram_visitor = typename diagram_visitor_t<DiagramConfig>::type;

    auto model = clanguml::common::generators::generate<diagram_model,
        diagram_config, diagram_visitor>(db, diagram->name,
        dynamic_cast<diagram_config &>(*diagram), translation_units, verbose);

    for (const auto generator_type : generators) {
        if (generator_type == generator_type_t::plantuml) {
            generate_diagram_select_generator<diagram_config,
                plantuml_generator_tag>(od, name, diagram, model);
        }
        else if (generator_type == generator_type_t::json) {
            generate_diagram_select_generator<diagram_config,
                json_generator_tag>(od, name, diagram, model);
        }
    }
}
} // namespace detail

void generate_diagram(const std::string &od, const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const clang::tooling::CompilationDatabase &db,
    const std::vector<std::string> &translation_units,
    const std::vector<clanguml::common::generator_type_t> &generators,
    bool verbose)
{
    using clanguml::common::generator_type_t;
    using clanguml::common::model::diagram_t;

    using clanguml::config::class_diagram;
    using clanguml::config::include_diagram;
    using clanguml::config::package_diagram;
    using clanguml::config::sequence_diagram;

    if (diagram->type() == diagram_t::kClass) {
        detail::generate_diagram_impl<class_diagram>(
            od, name, diagram, db, translation_units, generators, verbose);
    }
    else if (diagram->type() == diagram_t::kSequence) {
        detail::generate_diagram_impl<sequence_diagram>(
            od, name, diagram, db, translation_units, generators, verbose);
    }
    else if (diagram->type() == diagram_t::kPackage) {
        detail::generate_diagram_impl<package_diagram>(
            od, name, diagram, db, translation_units, generators, verbose);
    }
    else if (diagram->type() == diagram_t::kInclude) {
        detail::generate_diagram_impl<include_diagram>(
            od, name, diagram, db, translation_units, generators, verbose);
    }
}

void generate_diagrams(const std::vector<std::string> &diagram_names,
    clanguml::config::config &config, const std::string &od,
    const std::unique_ptr<clang::tooling::CompilationDatabase> &db,
    const int verbose, const unsigned int thread_count,
    const std::vector<clanguml::common::generator_type_t> &generators,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map)
{
    util::thread_pool_executor generator_executor{thread_count};
    std::vector<std::future<void>> futs;

    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command
        // line, and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        const auto &valid_translation_units = translation_units_map.at(name);

        if (valid_translation_units.empty()) {
            LOG_ERROR("Diagram {} generation failed: no translation units "
                      "found. Please make sure that your 'glob' patterns match "
                      "at least 1 file in 'compile_commands.json'.",
                name);
            continue;
        }

        futs.emplace_back(generator_executor.add(
            [&od, &generators, &name = name, &diagram = diagram,
                db = std::ref(*db), translation_units = valid_translation_units,
                verbose]() {
                try {
                    generate_diagram(od, name, diagram, db, translation_units,
                        generators, verbose != 0);
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

void adjust_compilation_database(const clanguml::config::config &config,
    clang::tooling::CompilationDatabase &db)
{
    if (config.add_compile_flags && !config.add_compile_flags().empty()) {
        for (auto &compile_command : db.getAllCompileCommands()) {
            compile_command.CommandLine.insert(
                compile_command.CommandLine.begin() + 1,
                config.add_compile_flags().begin(),
                config.add_compile_flags().end());
        }
    }
}

} // namespace clanguml::common::generators