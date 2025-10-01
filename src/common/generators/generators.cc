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
    DiagramModel &model)
{
    using diagram_generator =
        typename diagram_generator_t<DiagramConfig, GeneratorTag>::type;

    if constexpr (!std::is_same_v<diagram_generator, not_supported>) {
        std::stringstream buffer;
        buffer << diagram_generator(
            dynamic_cast<DiagramConfig &>(*diagram), model);

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
auto generate_diagram_impl(const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const common::compilation_database &db,
    const std::vector<std::string> &translation_units,
    const cli::runtime_config &runtime_config, std::function<void()> &&progress)
{
    using diagram_config = DiagramConfig;
    using diagram_model = typename diagram_model_t<DiagramConfig>::type;
    using diagram_visitor = typename diagram_visitor_t<DiagramConfig>::type;

    return clanguml::common::generators::generate_diagram_model<diagram_model,
        diagram_config, diagram_visitor>(db, name,
        dynamic_cast<diagram_config &>(*diagram), translation_units,
        runtime_config.verbose, std::move(progress));
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

bool is_diagram_supported_by_generators(
    const std::vector<generator_type_t> &generators,
    model::diagram_t diagram_type)
{
    return std::any_of(generators.begin(), generators.end(),
        [diagram_type](const auto generator_type) {
            if (generator_type == generator_type_t::plantuml) {
                if (generator_supports_diagram_type<plantuml_generator_tag>(
                        diagram_type))
                    return true;
            }
            else if (generator_type == generator_type_t::json) {
                if (generator_supports_diagram_type<json_generator_tag>(
                        diagram_type))
                    return true;
            }
            else if (generator_type == generator_type_t::mermaid) {
                if (generator_supports_diagram_type<mermaid_generator_tag>(
                        diagram_type))
                    return true;
            }
            else if (generator_type == generator_type_t::graphml) {
                if (generator_supports_diagram_type<graphml_generator_tag>(
                        diagram_type))
                    return true;
            }

            return false;
        });
}

template <typename T>
using diagram_model_future = std::future<std::unique_ptr<T>>;

template <typename T>
using diagram_model_futures = std::vector<diagram_model_future<T>>;

using class_diagram_model_futures =
    diagram_model_futures<class_diagram::model::diagram>;
using sequence_diagram_model_futures =
    diagram_model_futures<sequence_diagram::model::diagram>;
using include_diagram_model_futures =
    diagram_model_futures<include_diagram::model::diagram>;
using package_diagram_model_futures =
    diagram_model_futures<package_diagram::model::diagram>;
using diagram_model_variant_collection = std::map<
    std::string /* diagram name */,
    std::variant<class_diagram_model_futures, sequence_diagram_model_futures,
        include_diagram_model_futures, package_diagram_model_futures>>;

template <typename DiagramConfig, typename DiagramModel>
void create_model_future(util::thread_pool_executor &generator_executor,
    diagram_model_variant_collection &diagram_models, const std::string &name,
    const std::string &tu, const common::compilation_database_ptr &db,
    const std::shared_ptr<config::diagram> diagram,
    const cli::runtime_config &runtime_config,
    std::shared_ptr<progress_indicator_base> indicator)
{
    std::function<std::unique_ptr<DiagramModel>()> generator =
        make_generator<DiagramConfig>(name, tu, db,
            dynamic_cast<DiagramConfig &>(*diagram), runtime_config, indicator);

    auto fut = generator_executor.add<std::unique_ptr<DiagramModel>>(
        std::move(generator));

    if (!std::holds_alternative<diagram_model_futures<DiagramModel>>(
            diagram_models[name]))
        diagram_models[name] = diagram_model_futures<DiagramModel>{};

    std::get<diagram_model_futures<DiagramModel>>(diagram_models[name])
        .emplace_back(std::move(fut));
}

template <typename T>
std::unique_ptr<T> combine_partial_diagram_models(
    diagram_model_futures<T> &futs)
{
    std::vector<std::unique_ptr<T>> models;
    for (auto &fut : futs) {
        models.emplace_back(fut.get());
    }

    auto it = models.begin();
    auto &first = *it;
    it++;
    for (; it != models.end(); it++) {
        first->append(std::move(*it->get()));
    }
    first->finalize();
    first->apply_filter();

    return std::move(first);
}

template <typename DiagramConfig, typename DiagramModel>
void generate_diagrams_by_type(std::shared_ptr<config::diagram> diagram_config,
    DiagramModel &model, const std::string &name,
    const cli::runtime_config &runtime_config)
{
    for (const auto generator_type : runtime_config.generators) {
        if (generator_type == generator_type_t::plantuml) {
            detail::generate_diagram_select_generator<DiagramConfig,
                plantuml_generator_tag>(
                runtime_config.output_directory, name, diagram_config, model);
        }
        else if (generator_type == generator_type_t::json) {
            detail::generate_diagram_select_generator<DiagramConfig,
                json_generator_tag>(
                runtime_config.output_directory, name, diagram_config, model);
        }
        else if (generator_type == generator_type_t::mermaid) {
            detail::generate_diagram_select_generator<DiagramConfig,
                mermaid_generator_tag>(
                runtime_config.output_directory, name, diagram_config, model);
        }
        else if (generator_type == generator_type_t::graphml) {
            detail::generate_diagram_select_generator<DiagramConfig,
                graphml_generator_tag>(
                runtime_config.output_directory, name, diagram_config, model);
        }
    }
}

template <typename T> bool is_model_ready(T &futs)
{
    using namespace std::chrono_literals;
    return std::all_of(futs.begin(), futs.end(), [](auto &fut) {
        return fut.valid() && fut.wait_for(5ms) == std::future_status::ready;
    });
}

int generate_diagrams(const std::vector<std::string> &diagram_names,
    config::config &config, const common::compilation_database_ptr &db,
    const cli::runtime_config &runtime_config,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map)
{
    util::thread_pool_executor generator_executor{runtime_config.thread_count};

    diagram_model_variant_collection diagram_models;

    std::shared_ptr<progress_indicator_base> indicator;

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
        if (!is_diagram_supported_by_generators(
                runtime_config.generators, diagram->type())) {
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

        if (indicator) {
            indicator->add_progress_bar(name, matching_commands_count,
                diagram_type_to_color(diagram->type()));
        }

        for (const auto &tu : valid_translation_units) {
            if (diagram->type() == model::diagram_t::kClass) {
                create_model_future<config::class_diagram,
                    class_diagram::model::diagram>(generator_executor,
                    diagram_models, name, tu, db, diagram, runtime_config,
                    indicator);
            }
            else if (diagram->type() == model::diagram_t::kSequence) {
                create_model_future<config::sequence_diagram,
                    sequence_diagram::model::diagram>(generator_executor,
                    diagram_models, name, tu, db, diagram, runtime_config,
                    indicator);
            }
            else if (diagram->type() == model::diagram_t::kPackage) {
                create_model_future<config::package_diagram,
                    package_diagram::model::diagram>(generator_executor,
                    diagram_models, name, tu, db, diagram, runtime_config,
                    indicator);
            }
            else if (diagram->type() == model::diagram_t::kInclude) {
                create_model_future<config::include_diagram,
                    include_diagram::model::diagram>(generator_executor,
                    diagram_models, name, tu, db, diagram, runtime_config,
                    indicator);
            }
        }
    }

    LOG_INFO("Collecting diagram futures");

    size_t completed_diagrams{0};

    while (completed_diagrams < diagram_models.size()) {
        for (auto &[name, futs_variant] : diagram_models) {
            try {
                if (std::holds_alternative<class_diagram_model_futures>(
                        futs_variant)) {
                    auto &futs =
                        std::get<class_diagram_model_futures>(futs_variant);
                    if (!is_model_ready(futs))
                        continue;

                    auto combined_model = combine_partial_diagram_models<
                        class_diagram::model::diagram>(futs);

                    generate_diagrams_by_type<config::class_diagram,
                        class_diagram::model::diagram>(config.diagrams.at(name),
                        *combined_model, name, runtime_config);
                }
                else if (std::holds_alternative<sequence_diagram_model_futures>(
                             futs_variant)) {
                    auto &futs =
                        std::get<sequence_diagram_model_futures>(futs_variant);
                    if (!is_model_ready(futs))
                        continue;

                    auto combined_model = combine_partial_diagram_models<
                        sequence_diagram::model::diagram>(futs);

                    generate_diagrams_by_type<config::sequence_diagram,
                        sequence_diagram::model::diagram>(
                        config.diagrams.at(name), *combined_model, name,
                        runtime_config);
                }
                else if (std::holds_alternative<include_diagram_model_futures>(
                             futs_variant)) {
                    auto &futs =
                        std::get<include_diagram_model_futures>(futs_variant);
                    if (!is_model_ready(futs))
                        continue;

                    auto combined_model = combine_partial_diagram_models<
                        include_diagram::model::diagram>(futs);

                    generate_diagrams_by_type<config::include_diagram,
                        include_diagram::model::diagram>(
                        config.diagrams.at(name), *combined_model, name,
                        runtime_config);
                }
                else if (std::holds_alternative<package_diagram_model_futures>(
                             futs_variant)) {
                    auto &futs =
                        std::get<package_diagram_model_futures>(futs_variant);
                    if (!is_model_ready(futs))
                        continue;

                    auto combined_model = combine_partial_diagram_models<
                        package_diagram::model::diagram>(futs);

                    generate_diagrams_by_type<config::package_diagram,
                        package_diagram::model::diagram>(
                        config.diagrams.at(name), *combined_model, name,
                        runtime_config);
                }

                if (indicator)
                    indicator->complete(name);

                completed_diagrams++;
            }
            catch (clanguml::generators::clang_tool_exception &e) {
                if (indicator)
                    indicator->fail(name);
                completed_diagrams++;
                throw std::move(e);
            }
            catch (std::exception &e) {
                if (indicator)
                    indicator->fail(name);
                errors.emplace_back(std::current_exception());

                completed_diagrams++;
            }
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