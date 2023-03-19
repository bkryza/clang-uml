/**
 * src/common/generators/generators.h
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

#include "class_diagram/generators/json/class_diagram_generator.h"
#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "cli/cli_handler.h"
#include "common/generators/generators.h"
#include "common/model/diagram_filter.h"
#include "config/config.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "sequence_diagram/generators/json/sequence_diagram_generator.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "util/util.h"
#include "version.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CompilationDatabase.h>
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

void find_translation_units_for_diagrams(
    const std::vector<std::string> &diagram_names,
    clanguml::config::config &config,
    const std::vector<std::string> &compilation_database_files,
    std::map<std::string, std::vector<std::string>> &translation_units_map);

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

    void HandleTranslationUnit(clang::ASTContext &ast_context) override
    {
        visitor_.TraverseDecl(ast_context.getTranslationUnitDecl());
        visitor_.finalize();
    }
};

template <typename DiagramModel, typename DiagramConfig,
    typename DiagramVisitor>
class diagram_fronted_action : public clang::ASTFrontendAction {
public:
    explicit diagram_fronted_action(
        DiagramModel &diagram, const DiagramConfig &config)
        : diagram_{diagram}
        , config_{config}
    {
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &CI, clang::StringRef /*file*/) override
    {
        return std::make_unique<
            diagram_ast_consumer<DiagramModel, DiagramConfig, DiagramVisitor>>(
            CI, diagram_, config_);
    }

protected:
    bool BeginSourceFileAction(clang::CompilerInstance &ci) override
    {
        LOG_DBG("Visiting source file: {}", getCurrentFile().str());

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
};

template <typename DiagramModel, typename DiagramConfig,
    typename DiagramVisitor>
class diagram_action_visitor_factory
    : public clang::tooling::FrontendActionFactory {
public:
    explicit diagram_action_visitor_factory(
        DiagramModel &diagram, const DiagramConfig &config)
        : diagram_{diagram}
        , config_{config}
    {
    }

    std::unique_ptr<clang::FrontendAction> create() override
    {
        return std::make_unique<diagram_fronted_action<DiagramModel,
            DiagramConfig, DiagramVisitor>>(diagram_, config_);
    }

private:
    DiagramModel &diagram_;
    const DiagramConfig &config_;
};

template <typename DiagramModel, typename DiagramConfig,
    typename DiagramVisitor>
std::unique_ptr<DiagramModel> generate(
    const clang::tooling::CompilationDatabase &db, const std::string &name,
    DiagramConfig &config, const std::vector<std::string> &translation_units,
    bool /*verbose*/ = false)
{
    LOG_INFO("Generating diagram {}.puml", name);

    auto diagram = std::make_unique<DiagramModel>();
    diagram->set_name(name);
    diagram->set_filter(
        std::make_unique<model::diagram_filter>(*diagram, config));

    LOG_DBG("Found translation units for diagram {}: {}", name,
        fmt::join(translation_units, ", "));

    clang::tooling::ClangTool clang_tool(db, translation_units);
    auto action_factory =
        std::make_unique<diagram_action_visitor_factory<DiagramModel,
            DiagramConfig, DiagramVisitor>>(*diagram, config);

    auto res = clang_tool.run(action_factory.get());

    if (res != 0) {
        throw std::runtime_error("Diagram " + name + " generation failed");
    }

    diagram->set_complete(true);

    return diagram;
}

void generate_diagram(const std::string &od, const std::string &name,
    std::shared_ptr<clanguml::config::diagram> diagram,
    const clang::tooling::CompilationDatabase &db,
    const std::vector<std::string> &translation_units,
    const std::vector<clanguml::common::generator_type_t> &generators,
    bool verbose);

void generate_diagrams(const std::vector<std::string> &diagram_names,
    clanguml::config::config &config, const std::string &od,
    const std::unique_ptr<clang::tooling::CompilationDatabase> &db, int verbose,
    unsigned int thread_count,
    const std::vector<clanguml::common::generator_type_t> &generators,
    const std::map<std::string, std::vector<std::string>>
        &translation_units_map);

} // namespace clanguml::common::generators