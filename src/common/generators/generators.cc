/**
 * @file src/common/generators/generators.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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
void make_context_source_relative(
    inja::json &context, const std::string &prefix)
{
    if (!context.contains("element"))
        return;

    if (!context["element"].contains("source"))
        return;

    auto &source = context["element"]["source"];

    if (source.at("path").empty())
        return;

    auto path = std::filesystem::path{source.at("path").get<std::string>()};
    auto prefix_path = std::filesystem::path(prefix);
    if (path.is_absolute() && util::is_relative_to(path, prefix_path)) {
        source["path"] = relative(path, prefix_path);
        return;
    }
}

void find_translation_units_for_diagrams(
    const std::vector<std::string> &diagram_names,
    clanguml::config::config &config,
    const compilation_database &compilation_database,
    std::map<std::string, std::vector<std::string>> &translation_units_map)
{
    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command line,
        // and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        translation_units_map[name] =
            diagram->glob_translation_units(compilation_database.getAllFiles(),
                compilation_database.is_fixed());

        LOG_DBG("Found {} translation units for diagram '{}'",
            translation_units_map.at(name).size(), name);
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

    if constexpr (!std::is_same_v<diagram_generator, not_supported>) {

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
    else {
        LOG_INFO("Serialization to {} not supported for {}",
            GeneratorTag::extension, name);
    }
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

            if (logging::logger_type() == logging::logger_type_t::text) {
                for (const auto &from : from_values) {
                    std::cout << from << '\n';
                }
            }
            else {
                inja::json j = inja::json::array();
                for (const auto &from : from_values) {
                    j.emplace_back(logging::escape_json(from));
                }
                std::cout << j.dump();
            }

            return;
        }
        if (runtime_config.print_to) {
            auto to_values = model->list_to_values();
            if (logging::logger_type() == logging::logger_type_t::text) {
                for (const auto &to : to_values) {
                    std::cout << to << '\n';
                }
            }
            else {
                inja::json j = inja::json::array();
                for (const auto &to : to_values) {
                    j.emplace_back(logging::escape_json(to));
                }
                std::cout << j.dump();
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
        else if (generator_type == generator_type_t::graphml) {
            generate_diagram_select_generator<diagram_config,
                graphml_generator_tag>(
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

int generate_diagrams(const std::vector<std::string> &diagram_names,
    config::config &config, const common::compilation_database_ptr &db,
    const cli::runtime_config &runtime_config,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map)
{
    util::thread_pool_executor generator_executor{runtime_config.thread_count};
    std::vector<std::future<void>> futs;

    std::unique_ptr<progress_indicator_base> indicator;

    if (runtime_config.progress) {
        if (clanguml::logging::logger_type() == logging::logger_type_t::text) {
            std::cout
                << termcolor::white
                << "Processing translation units and generating diagrams:\n";
            indicator = std::make_unique<progress_indicator>();
        }
        else {
            indicator = std::make_unique<json_logger_progress_indicator>();
        }
    }

    std::vector<std::exception_ptr> errors;

    for (const auto &[name, diagram] : config.diagrams) {
        // If there are any specific diagram names provided on the command
        // line, and this diagram is not in that list - skip it
        if (!diagram_names.empty() && !util::contains(diagram_names, name))
            continue;

        // If none of the generators supports the diagram type - skip it
        bool at_least_one_generator_supports_diagram_type{false};
        for (const auto generator_type : runtime_config.generators) {
            if (generator_type == generator_type_t::plantuml) {
                if (generator_supports_diagram_type<plantuml_generator_tag>(
                        diagram->type()))
                    at_least_one_generator_supports_diagram_type = true;
            }
            else if (generator_type == generator_type_t::json) {
                if (generator_supports_diagram_type<json_generator_tag>(
                        diagram->type()))
                    at_least_one_generator_supports_diagram_type = true;
            }
            else if (generator_type == generator_type_t::mermaid) {
                if (generator_supports_diagram_type<mermaid_generator_tag>(
                        diagram->type()))
                    at_least_one_generator_supports_diagram_type = true;
            }
            else if (generator_type == generator_type_t::graphml) {
                if (generator_supports_diagram_type<graphml_generator_tag>(
                        diagram->type()))
                    at_least_one_generator_supports_diagram_type = true;
            }
        }

        if (!at_least_one_generator_supports_diagram_type) {
            LOG_INFO("Diagram '{}' not supported by any of selected "
                     "generators - skipping...",
                name);
            continue;
        }

        const auto &valid_translation_units = translation_units_map.at(name);

        LOG_DBG("Found {} possible translation units for diagram '{}'",
            valid_translation_units.size(), name);

        const auto matching_commands_count =
            db->count_matching_commands(valid_translation_units);

        if (matching_commands_count == 0) {
            const auto error_msg = fmt::format(
                "Diagram '{}' generation failed: no translation units "
                "found. Please make sure that your 'glob' patterns match "
                "at least 1 file in 'compile_commands.json'.",
                name);

            if (indicator) {
                indicator->add_progress_bar(
                    name, 0, diagram_type_to_color(diagram->type()));
                indicator->fail(name);
                try {
                    throw std::runtime_error(error_msg);
                }
                catch (std::runtime_error &e) {
                    errors.emplace_back(std::current_exception());
                }
            }
            else {
                LOG_ERROR(error_msg);
            }

            continue;
        }

        LOG_DBG("Found {} matching translation unit commands for diagram {}",
            matching_commands_count, name);

        auto generator = [&name = name, &diagram = diagram, &indicator,
                             db = std::ref(*db), matching_commands_count,
                             translation_units = valid_translation_units,
                             runtime_config]() mutable -> void {
            try {
                if (indicator) {
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
                else {
                    generate_diagram(name, diagram, db, translation_units,
                        runtime_config, {});
                }
            }
            catch (clanguml::generators::clang_tool_exception &e) {
                if (indicator)
                    indicator->fail(name);
                throw std::move(e);
            }
            catch (std::exception &e) {
                if (indicator)
                    indicator->fail(name);

                LOG_ERROR(
                    "Failed to generate diagram '{}': {}", name, e.what());

                throw std::runtime_error(fmt::format(
                    "Failed to generate diagram '{}': {}", name, e.what()));
            }
        };

        futs.emplace_back(generator_executor.add(std::move(generator)));
    }

    for (auto &fut : futs) {
        try {
            fut.get();
        }
        catch (std::exception &e) {
            errors.emplace_back(std::current_exception());
        }
    }

    if (runtime_config.progress &&
        clanguml::logging::logger_type() == logging::logger_type_t::text) {
        indicator->stop();
        if (errors.empty()) {
            std::cout << termcolor::white << "Done\n";
        }
        else {
            std::cout << termcolor::white << "\n";
        }
        std::cout << termcolor::reset;
    }

    if (errors.empty())
        return 0;

    for (auto &e : errors) {
        try {
            std::rethrow_exception(e);
        }
        catch (const clanguml::generators::clang_tool_exception &e) {
            if (clanguml::logging::logger_type() ==
                logging::logger_type_t::text) {

                fmt::println("ERROR: Failed to generate {} diagram '{}' due to "
                             "following issues:",
                    e.diagram_type(), e.diagram_name());
                for (const auto &d : e.diagnostics) {
                    fmt::println(" - {}", d);
                }
                fmt::println("");
            }
            else {
                inja::json j;
                j["diagram_name"] = e.diagram_name();
                j["clang_errors"] = inja::json::array();
                for (const auto &d : e.diagnostics) {
                    j["clang_errors"].emplace_back(d);
                }

                spdlog::get("clanguml-logger")
                    ->log(spdlog::level::err,
                        fmt::runtime(
                            R"("file": "{}", "line": {}, "message": {})"),
                        FILENAME_, __LINE__, j.dump());
            }
        }
        catch (const std::exception &e) {
            if (clanguml::logging::logger_type() ==
                logging::logger_type_t::text) {
                fmt::println("ERROR: {}", e.what());
            }
            else {
                LOG_ERROR("{}", e.what());
            }
        }
    }

    return 1;
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