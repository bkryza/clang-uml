/**
 * @file src/common/generators/generators.h
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

#include "class_diagram/generators/json/class_diagram_generator.h"
#include "class_diagram/generators/mermaid/class_diagram_generator.h"
#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "cli/cli_handler.h"
#include "common/compilation_database.h"
#include "common/model/diagram_filter.h"
#include "config/config.h"
#include "include_diagram/generators/json/include_diagram_generator.h"
#include "include_diagram/generators/mermaid/include_diagram_generator.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "indicators/indicators.hpp"
#include "package_diagram/generators/json/package_diagram_generator.h"
#include "package_diagram/generators/mermaid/package_diagram_generator.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "sequence_diagram/generators/json/sequence_diagram_generator.h"
#include "sequence_diagram/generators/mermaid/sequence_diagram_generator.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "util/util.h"
#include "version.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <string>
#include <util/thread_pool_executor.h>
#include <vector>

namespace clanguml::common::generators {

/** @defgroup diagram_model_t Diagram model selector
 *
 * Template traits for selecting diagram model type based on diagram config
 * type
 *
 * @{
 */
template <typename DiagramConfig> struct diagram_model_t;
template <> struct diagram_model_t<clanguml::config::class_diagram> {
    using type = clanguml::class_diagram::model::diagram;
};
template <> struct diagram_model_t<clanguml::config::sequence_diagram> {
    using type = clanguml::sequence_diagram::model::diagram;
};
template <> struct diagram_model_t<clanguml::config::package_diagram> {
    using type = clanguml::package_diagram::model::diagram;
};
template <> struct diagram_model_t<clanguml::config::include_diagram> {
    using type = clanguml::include_diagram::model::diagram;
};
/** @} */

/** @defgroup diagram_visitor_t Diagram model selector
 *
 * Template traits for selecting diagram visitor type based on diagram config
 * type
 *
 * @{
 */
template <typename DiagramConfig> struct diagram_visitor_t;
template <> struct diagram_visitor_t<clanguml::config::class_diagram> {
    using type = clanguml::class_diagram::visitor::translation_unit_visitor;
};
template <> struct diagram_visitor_t<clanguml::config::sequence_diagram> {
    using type = clanguml::sequence_diagram::visitor::translation_unit_visitor;
};
template <> struct diagram_visitor_t<clanguml::config::package_diagram> {
    using type = clanguml::package_diagram::visitor::translation_unit_visitor;
};
template <> struct diagram_visitor_t<clanguml::config::include_diagram> {
    using type = clanguml::include_diagram::visitor::translation_unit_visitor;
};
/** @} */

/** @defgroup diagram_generator_tag Diagram model tags
 *
 * Tags to determine the generator output file extension
 *
 * @{
 */
struct plantuml_generator_tag {
    inline static const std::string extension = "puml";
};
struct json_generator_tag {
    inline static const std::string extension = "json";
};
struct mermaid_generator_tag {
    inline static const std::string extension = "mmd";
};
/** @} */

/** @defgroup diagram_generator_t Diagram generator selector
 *
 * Tags to determine the generator type based on diagram config type
 * and output format
 *
 * @{
 */
// plantuml
template <typename DiagramConfig, typename GeneratorType>
struct diagram_generator_t;
template <>
struct diagram_generator_t<clanguml::config::class_diagram,
    plantuml_generator_tag> {
    using type = clanguml::class_diagram::generators::plantuml::generator;
};
template <>
struct diagram_generator_t<clanguml::config::sequence_diagram,
    plantuml_generator_tag> {
    using type = clanguml::sequence_diagram::generators::plantuml::generator;
};
template <>
struct diagram_generator_t<clanguml::config::package_diagram,
    plantuml_generator_tag> {
    using type = clanguml::package_diagram::generators::plantuml::generator;
};
template <>
struct diagram_generator_t<clanguml::config::include_diagram,
    plantuml_generator_tag> {
    using type = clanguml::include_diagram::generators::plantuml::generator;
};
// json
template <>
struct diagram_generator_t<clanguml::config::class_diagram,
    json_generator_tag> {
    using type = clanguml::class_diagram::generators::json::generator;
};
template <>
struct diagram_generator_t<clanguml::config::sequence_diagram,
    json_generator_tag> {
    using type = clanguml::sequence_diagram::generators::json::generator;
};
template <>
struct diagram_generator_t<clanguml::config::package_diagram,
    json_generator_tag> {
    using type = clanguml::package_diagram::generators::json::generator;
};
template <>
struct diagram_generator_t<clanguml::config::include_diagram,
    json_generator_tag> {
    using type = clanguml::include_diagram::generators::json::generator;
};
// mermaid
template <>
struct diagram_generator_t<clanguml::config::class_diagram,
    mermaid_generator_tag> {
    using type = clanguml::class_diagram::generators::mermaid::generator;
};
template <>
struct diagram_generator_t<clanguml::config::sequence_diagram,
    mermaid_generator_tag> {
    using type = clanguml::sequence_diagram::generators::mermaid::generator;
};
template <>
struct diagram_generator_t<clanguml::config::package_diagram,
    mermaid_generator_tag> {
    using type = clanguml::package_diagram::generators::mermaid::generator;
};
template <>
struct diagram_generator_t<clanguml::config::include_diagram,
    mermaid_generator_tag> {
    using type = clanguml::include_diagram::generators::mermaid::generator;
};
/** @} */

/**
 * @brief Assign translation units to diagrams
 *
 * This function assigns for each diagram to be generated the list of
 * translation units based on it's `glob` pattern if any.
 *
 * If `diagram_names` is empty, this function processes all diagrams in
 * `config`.
 *
 * @param diagram_names List of diagram names, applies to all if empty
 * @param config Reference to config instance
 * @param compilation_database_files List of files found in compilation database
 * @param translation_units_map Resulting translation units map is stored here
 */
void find_translation_units_for_diagrams(
    const std::vector<std::string> &diagram_names,
    clanguml::config::config &config,
    const std::vector<std::string> &compilation_database_files,
    std::map<std::string, std::vector<std::string>> &translation_units_map);

/**
 * @brief Specialization of
 * [clang::ASTConsumer](https://clang.llvm.org/doxygen/classclang_1_1ASTConsumer.html)
 *
 * This class provides overriden HandleTranslationUnit() method, which
 * calls a translation_unit_visitor for a specific diagram type on
 * each translation unit assigned to the diagram.
 *
 * @tparam DiagramModel Type of diagram_model
 * @tparam DiagramConfig Type of diagram_config
 * @tparam TranslationUnitVisitor Type of translation_unit_visitor
 */
template <typename DiagramModel, typename DiagramConfig,
    typename TranslationUnitVisitor>
class diagram_ast_consumer : public clang::ASTConsumer {
    TranslationUnitVisitor visitor_;

public:
    explicit diagram_ast_consumer(clang::CompilerInstance &ci,
        DiagramModel &diagram, const DiagramConfig &config)
        : visitor_{ci.getSourceManager(), diagram, config}
    {
    }

    TranslationUnitVisitor &visitor() { return visitor_; }

    void HandleTranslationUnit(clang::ASTContext &ast_context) override
    {
        visitor_.TraverseDecl(ast_context.getTranslationUnitDecl());
        visitor_.finalize();
    }
};

/**
 * @brief Specialization of
 * [clang::ASTFrontendAction](https://clang.llvm.org/doxygen/classclang_1_1ASTFrontendAction.html)
 *
 * This class overrides the BeginSourceFileAction() and CreateASTConsumer()
 * methods to create and setup an appropriate diagram_ast_consumer instance.
 *
 * @tparam DiagramModel Type of diagram_model
 * @tparam DiagramConfig Type of diagram_config
 * @tparam TranslationUnitVisitor Type of translation_unit_visitor
 */
template <typename DiagramModel, typename DiagramConfig,
    typename DiagramVisitor>
class diagram_fronted_action : public clang::ASTFrontendAction {
public:
    explicit diagram_fronted_action(DiagramModel &diagram,
        const DiagramConfig &config, std::function<void()> progress)
        : diagram_{diagram}
        , config_{config}
        , progress_{std::move(progress)}
    {
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &CI, clang::StringRef /*file*/) override
    {
        auto ast_consumer = std::make_unique<
            diagram_ast_consumer<DiagramModel, DiagramConfig, DiagramVisitor>>(
            CI, diagram_, config_);

        if constexpr (!std::is_same_v<DiagramModel,
                          clanguml::include_diagram::model::diagram>) {
            ast_consumer->visitor().set_tu_path(getCurrentFile().str());
        }

        return ast_consumer;
    }

protected:
    bool BeginSourceFileAction(clang::CompilerInstance &ci) override
    {
        LOG_DBG("Visiting source file: {}", getCurrentFile().str());

        // Update progress indicators, if enabled, on each translation
        // unit
        if (progress_)
            progress_();

        if constexpr (std::is_same_v<DiagramModel,
                          clanguml::include_diagram::model::diagram>) {
            auto find_includes_callback =
                std::make_unique<typename DiagramVisitor::include_visitor>(
                    ci.getSourceManager(), diagram_, config_);

            clang::Preprocessor &pp = ci.getPreprocessor();

            pp.addPPCallbacks(std::move(find_includes_callback));
        }

        return true;
    }

private:
    DiagramModel &diagram_;
    const DiagramConfig &config_;
    std::function<void()> progress_;
};

/**
 * @brief Specialization of
 * [clang::ASTFrontendAction](https://clang.llvm.org/doxygen/classclang_1_1tooling_1_1FrontendActionFactory.html)
 *
 * This class overrides the create() method in order to create an instance
 * of diagram_frontend_action of appropriate type.
 *
 * @tparam DiagramModel Type of diagram_model
 * @tparam DiagramConfig Type of diagram_config
 * @tparam TranslationUnitVisitor Type of translation_unit_visitor
 */
template <typename DiagramModel, typename DiagramConfig,
    typename DiagramVisitor>
class diagram_action_visitor_factory
    : public clang::tooling::FrontendActionFactory {
public:
    explicit diagram_action_visitor_factory(DiagramModel &diagram,
        const DiagramConfig &config, std::function<void()> progress)
        : diagram_{diagram}
        , config_{config}
        , progress_{std::move(progress)}
    {
    }

    std::unique_ptr<clang::FrontendAction> create() override
    {
        return std::make_unique<diagram_fronted_action<DiagramModel,
            DiagramConfig, DiagramVisitor>>(diagram_, config_, progress_);
    }

private:
    DiagramModel &diagram_;
    const DiagramConfig &config_;
    std::function<void()> progress_;
};

/**
 * @brief Specialization of
 * [clang::ASTFrontendAction](https://clang.llvm.org/doxygen/classclang_1_1tooling_1_1FrontendActionFactory.html)
 *
 * This is the entry point function to initiate AST frontend action for a
 * specific diagram.
 *
 * @embed{diagram_generate_generic_sequence.svg}
 *
 * @tparam DiagramModel Type of diagram_model
 * @tparam DiagramConfig Type of diagram_config
 * @tparam TranslationUnitVisitor Type of translation_unit_visitor
 */
template <typename DiagramModel, typename DiagramConfig,
    typename DiagramVisitor>
std::unique_ptr<DiagramModel> generate(const common::compilation_database &db,
    const std::string &name, DiagramConfig &config,
    const std::vector<std::string> &translation_units, bool /*verbose*/ = false,
    std::function<void()> progress = {})
{
    LOG_INFO("Generating diagram {}", name);

    auto diagram = std::make_unique<DiagramModel>();
    diagram->set_name(name);
    diagram->set_filter(
        std::make_unique<model::diagram_filter>(*diagram, config));

    LOG_DBG("Found translation units for diagram {}: {}", name,
        fmt::join(translation_units, ", "));

    clang::tooling::ClangTool clang_tool(db, translation_units);
    auto action_factory =
        std::make_unique<diagram_action_visitor_factory<DiagramModel,
            DiagramConfig, DiagramVisitor>>(
            *diagram, config, std::move(progress));

    auto res = clang_tool.run(action_factory.get());

    if (res != 0) {
        throw std::runtime_error("Diagram " + name + " generation failed");
    }

    diagram->set_complete(true);

    diagram->finalize();

    return diagram;
}

/**
 * @brief Generate a single diagram
 *
 * @param name Name of the diagram
 * @param diagram Effective diagram configuration
 * @param db Reference to compilation database
 * @param translation_units List of translation units for the diagram
 * @param generators List of generator types to be used for the diagram
 * @param verbose Log level
 * @param progress Function to report translation unit progress
 */
void generate_diagram(const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const common::compilation_database &db,
    const std::vector<std::string> &translation_units,
    const cli::runtime_config &runtime_config,
    std::function<void()> &&progress);

/**
 * @brief Generate diagrams
 *
 * @param diagram_names List of diagram names to generate
 * @param config Reference to config instance
 * @param output_directory Path to output directory
 * @param db Reference to compilation database
 * @param verbose Log level
 * @param thread_count Number of diagrams to be generated in parallel
 * @param progress Whether progress indicators should be displayed
 * @param generators List of generator types to use for each diagram
 * @param translation_units_map Map of translation units for each file
 */
void generate_diagrams(const std::vector<std::string> &diagram_names,
    clanguml::config::config &config,
    const common::compilation_database_ptr &db,
    const cli::runtime_config &runtime_config,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map);

/**
 * @brief Return indicators progress bar color for diagram type
 *
 * @param diagram_type Diagram type
 * @return Progress bar color
 */
indicators::Color diagram_type_to_color(model::diagram_t diagram_type);

} // namespace clanguml::common::generators
