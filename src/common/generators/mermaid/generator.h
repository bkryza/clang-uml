/**
 * @file src/common/generators/mermaid/generator.h
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

#include "common/generators/generator.h"
#include "common/model/filters/diagram_filter.h"
#include "config/config.h"
#include "util/error.h"
#include "util/util.h"
#include "version/version.h"

#include <clang/Basic/Version.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <glob/glob.hpp>
#include <inja/inja.hpp>

namespace clanguml::common::generators::mermaid {

using clanguml::common::model::access_t;
using clanguml::common::model::element;
using clanguml::common::model::message_t;
using clanguml::common::model::relationship_t;

std::string to_mermaid(relationship_t r);
std::string to_mermaid(access_t scope);
std::string to_mermaid(message_t r);

std::string indent(unsigned level);

std::string render_name(std::string name);
std::string escape_name(std::string name, bool round_brackets = true);

/**
 * @brief Base class for diagram generators
 *
 * @tparam ConfigType Configuration type
 * @tparam DiagramType Diagram model type
 */
template <typename ConfigType, typename DiagramType>
class generator
    : public clanguml::common::generators::generator<ConfigType, DiagramType> {
public:
    /**
     * @brief Constructor
     *
     * @param config Reference to instance of @link clanguml::config::diagram
     * @param model Reference to instance of @link clanguml::model::diagram
     */
    generator(ConfigType &config, DiagramType &model)
        : clanguml::common::generators::generator<ConfigType, DiagramType>{
              config, model}
    {
    }

    ~generator() override = default;

    /**
     * @brief Generate diagram
     *
     * This is the main diagram generation entrypoint. It is responsible for
     * calling other methods in appropriate order to generate the diagram into
     * the output stream. It generates diagram elements, that are common
     * to all types of diagrams in a given generator.
     *
     * @param ostr Output stream
     */
    void generate(std::ostream &ostr) const override;

    /**
     * @brief Generate diagram specific part
     *
     * This method must be implemented in subclasses for specific diagram
     * types.
     *
     * @param ostr Output stream
     */
    virtual void generate_diagram(std::ostream &ostr) const = 0;

    /**
     * @brief Generate MermaidJS directives from config file.
     *
     * This method renders the MermaidJS directives provided in the
     * configuration file, including resolving any element aliases and Jinja
     * templates.
     *
     * @param ostr Output stream
     * @param directives List of directives from the configuration file
     */
    void generate_mermaid_directives(
        std::ostream &ostr, const std::vector<std::string> &directives) const;

    /**
     * @brief Generate the diagram type
     *
     * This method must be overriden for each diagram type (e.g. it renders
     * a single line `classDiagram` for Mermaid class diagrams.
     *
     * @param ostr Output stream
     */
    virtual void generate_diagram_type(std::ostream &ostr) const = 0;

    /**
     * @brief Generate diagram notes
     *
     * This method adds any notes in the diagram, which were declared in the
     * code using inline directives
     *
     * @param ostr Output stream
     * @param element Element to which the note should be attached
     */
    virtual void generate_notes(
        std::ostream &ostr, const model::diagram_element &element) const;

    /**
     * @brief Generate comment with diagram metadata
     *
     * @param ostr Output stream
     */
    void generate_metadata(std::ostream &ostr) const;

    /**
     * @brief Generate diagram title
     *
     * Generates a MermaidJS diagram title directive if diagram title
     * is provided in the diagram configuration.
     *
     * @param ostr Output stream
     */
    void generate_title(std::ostream &ostr) const;

    /**
     * @brief Generate hyper link to element
     *
     * This method renders links to URL's based on templates provided
     * in the configuration file (e.g. Git browser with specific line and
     * column offset)
     *
     * @param ostr Output stream
     * @param e Reference to diagram element
     * @tparam E Diagram element type
     */
    template <typename E>
    void generate_link(std::ostream &ostr, const E &e) const;

    /**
     * @brief Print debug information in diagram comments
     *
     * @param m Diagram element to describe
     * @param ostr Output stream
     */
    void print_debug(
        const common::model::source_location &e, std::ostream &ostr) const;

protected:
    mutable std::set<std::string> m_generated_aliases;
};

template <typename C, typename D>
void generator<C, D>::generate(std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();
    const auto &model = generators::generator<C, D>::model();

    if (!config.allow_empty_diagrams() && model.is_empty() &&
        config.mermaid().before.empty() && config.mermaid().after.empty()) {
        throw clanguml::error::empty_diagram_error{model.type(), model.name(),
            "Diagram configuration resulted in empty diagram."};
    }

    generators::generator<C, D>::update_context();

    generate_title(ostr);

    generate_diagram_type(ostr);

    generate_mermaid_directives(ostr, config.mermaid().before);

    generate_diagram(ostr);

    generate_mermaid_directives(ostr, config.mermaid().after);

    generate_metadata(ostr);
}

template <typename C, typename D>
template <typename E>
void generator<C, D>::generate_link(std::ostream &ostr, const E &e) const
{
    if (e.file().empty() && e.file_relative().empty())
        return;

    auto maybe_link_pattern = generators::generator<C, D>::get_link_pattern(e);

    if (!maybe_link_pattern)
        return;

    const auto &[link_prefix, link_pattern] = *maybe_link_pattern;

    ostr << indent(1) << "click " << e.alias() << " href \"";
    try {
        auto ec = generators::generator<C, D>::element_context(e);
        common::generators::make_context_source_relative(ec, link_prefix);
        std::string link{};
        if (!link_pattern.empty()) {
            link = generators::generator<C, D>::env().render(
                std::string_view{link_pattern}, ec);
        }
        if (link.empty())
            link = " ";
        ostr << link;
    }
    catch (const inja::json::parse_error &e) {
        LOG_ERROR("Failed to parse Jinja template: {}", link_pattern);
        ostr << " ";
    }
    catch (const inja::json::exception &e) {
        LOG_ERROR("Failed to render comment directive: \n{}\n due to: {}",
            link_pattern, e.what());
        ostr << " ";
    }
    ostr << "\"";

    auto maybe_tooltip_pattern =
        generators::generator<C, D>::get_tooltip_pattern(e);

    if (maybe_tooltip_pattern) {
        const auto &[tooltip_prefix, tooltip_pattern] = *maybe_tooltip_pattern;

        if (!tooltip_pattern.empty()) {
            ostr << " \"";
            try {
                auto ec = generators::generator<C, D>::element_context(e);
                common::generators::make_context_source_relative(
                    ec, tooltip_prefix);
                auto tooltip_text = generators::generator<C, D>::env().render(
                    std::string_view{tooltip_pattern}, ec);
                util::replace_all(tooltip_text, "\"", "&bdquo;");
                ostr << tooltip_text;
            }
            catch (const inja::json::parse_error &e) {
                LOG_ERROR(
                    "Failed to parse Jinja template: {}", tooltip_pattern);
                ostr << " ";
            }
            catch (const inja::json::exception &e) {
                LOG_ERROR(
                    "Failed to render PlantUML directive: \n{}\n due to: {}",
                    tooltip_pattern, e.what());
                ostr << " ";
            }

            ostr << "\"";
        }
    }
    ostr << "\n";
}

template <typename C, typename D>
void generator<C, D>::generate_mermaid_directives(
    std::ostream &ostr, const std::vector<std::string> &directives) const
{
    using common::model::namespace_;

    for (const auto &d : directives) {
        auto rendered_directive =
            generators::generator<C, D>::render_template(d);
        if (rendered_directive)
            ostr << indent(1) << *rendered_directive << '\n';
    }
}

template <typename C, typename D>
void generator<C, D>::generate_notes(
    std::ostream &ostr, const model::diagram_element &e) const
{
    const auto &config = generators::generator<C, D>::config();

    for (const auto &decorator : e.decorators()) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(config.name)) {
            ostr << indent(1) << "note for " << e.alias() << " \"" << note->text
                 << "\"" << '\n';
        }
    }
}

template <typename C, typename D>
void generator<C, D>::generate_metadata(std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();

    if (config.generate_metadata()) {
        ostr << '\n'
             << "%% Generated with clang-uml, version "
             << clanguml::version::version() << '\n'
             << "%% LLVM version " << clang::getClangFullVersion() << '\n';
    }
}

template <typename C, typename D>
void generator<C, D>::generate_title(std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();

    if (config.title) {
        ostr << "---\n";
        ostr << "title: " << config.title() << '\n';
        ostr << "---\n";
    }
}

template <typename C, typename D>
void generator<C, D>::print_debug(
    const common::model::source_location &e, std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();

    if (config.debug_mode())
        ostr << "%% " << e.file() << ":" << e.line() << '\n';
}

template <typename DiagramModel, typename DiagramConfig>
std::ostream &operator<<(
    std::ostream &os, const generator<DiagramModel, DiagramConfig> &g)
{
    g.generate(os);
    return os;
}
} // namespace clanguml::common::generators::mermaid