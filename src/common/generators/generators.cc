/**
 * @file src/common/generators/generators.cc
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

#include "generators.h"

#include "progress_indicator.h"

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

void render_diagram(const clanguml::common::generator_type_t generator_type,
    std::shared_ptr<config::diagram> diagram_config)
{
    std::string cmd;
    switch (generator_type) {
    case clanguml::common::generator_type_t::plantuml:
        cmd = diagram_config->puml().cmd;
        break;
    case clanguml::common::generator_type_t::mermaid:
        cmd = diagram_config->mermaid().cmd;
        break;
    default:
        return;
    };

    if (cmd.empty())
        throw std::runtime_error(
            fmt::format("No render command template provided for {} diagrams",
                to_string(diagram_config->type())));

    util::replace_all(cmd, "{}", diagram_config->name);

    LOG_INFO("Rendering diagram {} using {}", diagram_config->name,
        to_string(generator_type));

    util::check_process_output(cmd);
}

namespace detail {

template <typename DiagramConfig, typename GeneratorTag, typename DiagramModel>
void generate_diagram_select_generator(const std::string &od,
    const std::string &name, std::shared_ptr<clanguml::config::diagram> diagram,
    const DiagramModel &model)
{
    using diagram_generator =
        typename diagram_generator_t<DiagramConfig, GeneratorTag>::type;

    std::stringstream buffer;
    buffer << diagram_generator(
        dynamic_cast<DiagramConfig &>(*diagram), *model);

    // Only open the file after the diagram has been generated successfully
    // in order not to overwrite previous diagram in case of failure
    auto path = std::filesystem::path{od} /
        fmt::format("{}.{}", name, GeneratorTag::extension);
    std::ofstream ofs;
    ofs.open(path, std::ofstream::out | std::ofstream::trunc);
    ofs << buffer.str();

    ofs.close();

    LOG_INFO("Written {} diagram to {}", name, path.string());
}

template <typename DiagramConfig>
void generate_diagram_impl(const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const common::compilation_database &db,
    const std::vector<std::string> &translation_units,
    const cli::runtime_config &runtime_config, std::function<void()> &&progress)
{
    using diagram_config = DiagramConfig;
    using diagram_model = typename diagram_model_t<DiagramConfig>::type;
    using diagram_visitor = typename diagram_visitor_t<DiagramConfig>::type;

    auto model = clanguml::common::generators::generate<diagram_model,
        diagram_config, diagram_visitor>(db, diagram->name,
        dynamic_cast<diagram_config &>(*diagram), translation_units,
        runtime_config.verbose, std::move(progress));

    if constexpr (std::is_same_v<DiagramConfig, config::sequence_diagram>) {
        if (runtime_config.print_from) {
            auto from_values = model->list_from_values();

            for (const auto &from : from_values) {
                std::cout << from << '\n';
            }

            return;
        }
        if (runtime_config.print_to) {
            auto to_values = model->list_to_values();

            for (const auto &to : to_values) {
                std::cout << "|" << to << "|" << '\n';
            }

            return;
        }
    }

    for (const auto generator_type : runtime_config.generators) {
        if (generator_type == generator_type_t::plantuml) {
            generate_diagram_select_generator<diagram_config,
                plantuml_generator_tag>(
                runtime_config.output_directory, name, diagram, model);
        }
        else if (generator_type == generator_type_t::json) {
            generate_diagram_select_generator<diagram_config,
                json_generator_tag>(
                runtime_config.output_directory, name, diagram, model);
        }
        else if (generator_type == generator_type_t::mermaid) {
            generate_diagram_select_generator<diagram_config,
                mermaid_generator_tag>(
                runtime_config.output_directory, name, diagram, model);
        }

        // Convert plantuml or mermaid to an image using command provided
        // in the command line arguments
        if (runtime_config.render_diagrams) {
            render_diagram(generator_type, diagram);
        }
    }
}
} // namespace detail

void generate_diagram(const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const common::compilation_database &db,
    const std::vector<std::string> &translation_units,
    const cli::runtime_config &runtime_config, std::function<void()> &&progress)
{
    using clanguml::common::generator_type_t;
    using clanguml::common::model::diagram_t;

    using clanguml::config::class_diagram;
    using clanguml::config::include_diagram;
    using clanguml::config::package_diagram;
    using clanguml::config::sequence_diagram;

    if (diagram->type() == diagram_t::kClass) {
        detail::generate_diagram_impl<class_diagram>(name, diagram, db,
            translation_units, runtime_config, std::move(progress));
    }
    else if (diagram->type() == diagram_t::kSequence) {
        detail::generate_diagram_impl<sequence_diagram>(name, diagram, db,
            translation_units, runtime_config, std::move(progress));
    }
    else if (diagram->type() == diagram_t::kPackage) {
        detail::generate_diagram_impl<package_diagram>(name, diagram, db,
            translation_units, runtime_config, std::move(progress));
    }
    else if (diagram->type() == diagram_t::kInclude) {
        detail::generate_diagram_impl<include_diagram>(name, diagram, db,
            translation_units, runtime_config, std::move(progress));
    }
}

void generate_diagrams(const std::vector<std::string> &diagram_names,
    config::config &config, const common::compilation_database_ptr &db,
    const cli::runtime_config &runtime_config,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map)
{
    util::thread_pool_executor generator_executor{runtime_config.thread_count};
    std::vector<std::future<void>> futs;

    std::unique_ptr<progress_indicator> indicator;

    if (runtime_config.progress) {
        std::cout << termcolor::white
                  << "Processing translation units and generating diagrams:\n";
        indicator = std::make_unique<progress_indicator>();
    }

    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command
        // line, and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        const auto &valid_translation_units = translation_units_map.at(name);

        if (valid_translation_units.empty()) {
            if (indicator) {
                indicator->add_progress_bar(
                    name, 0, diagram_type_to_color(diagram->type()));
                indicator->fail(name);
            }
            else {
                LOG_ERROR(
                    "Diagram {} generation failed: no translation units "
                    "found. Please make sure that your 'glob' patterns match "
                    "at least 1 file in 'compile_commands.json'.",
                    name);
            }
            continue;
        }

        const auto matching_commands_count =
            db->count_matching_commands(valid_translation_units);

        auto generator = [&name = name, &diagram = diagram, &indicator,
                             db = std::ref(*db), matching_commands_count,
                             translation_units = valid_translation_units,
                             runtime_config]() mutable {
            try {
                if (indicator)
                    indicator->add_progress_bar(name, matching_commands_count,
                        diagram_type_to_color(diagram->type()));

                generate_diagram(name, diagram, db, translation_units,
                    runtime_config, [&indicator, &name]() {
                        if (indicator)
                            indicator->increment(name);
                    });

                if (indicator)
                    indicator->complete(name);
            }
            catch (const std::exception &e) {
                if (indicator)
                    indicator->fail(name);

                LOG_ERROR(
                    "ERROR: Failed to generate diagram {}: {}", name, e.what());
            }
        };

        futs.emplace_back(generator_executor.add(std::move(generator)));
    }

    for (auto &fut : futs) {
        fut.get();
    }

    if (runtime_config.progress) {
        indicator->stop();
        std::cout << termcolor::white << "Done\n";
        std::cout << termcolor::reset;
    }
}

indicators::Color diagram_type_to_color(model::diagram_t diagram_type)
{
    switch (diagram_type) {
    case model::diagram_t::kClass:
        return indicators::Color::yellow;
    case model::diagram_t::kSequence:
        return indicators::Color::blue;
    case model::diagram_t::kPackage:
        return indicators::Color::cyan;
    case model::diagram_t::kInclude:
        return indicators::Color::magenta;
    default:
        return indicators::Color::unspecified;
    }
}

} // namespace clanguml::common::generators