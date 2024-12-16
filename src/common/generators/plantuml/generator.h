/**
 * @file src/common/generators/plantuml/generator.h
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
#include "common/model/relationship.h"
#include "config/config.h"
#include "util/util.h"
#include "version/version.h"

#include <clang/Basic/Version.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <glob/glob.hpp>
#include <inja/inja.hpp>

namespace clanguml::common::generators::plantuml {

using clanguml::common::model::access_t;
using clanguml::common::model::element;
using clanguml::common::model::message_t;
using clanguml::common::model::relationship;

std::string to_plantuml(const relationship &r, const config::diagram &cfg);
std::string to_plantuml(access_t scope);
std::string to_plantuml(message_t r);

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
     * @brief Generate diagram layout hints
     *
     * This method adds to the diagram any layout hints that were provided
     * in the configuration file.
     *
     * @param ostr Output stream
     */
    void generate_config_layout_hints(std::ostream &ostr) const;

    /**
     * @brief Generate PlantUML directives from config file.
     *
     * This method renders the PlantUML directives provided in the configuration
     * file, including resolving any element aliases and Jinja templates.
     *
     * @param ostr Output stream
     * @param directives List of directives from the configuration file
     */
    void generate_plantuml_directives(
        std::ostream &ostr, const std::vector<std::string> &directives) const;

    /**
     * @brief Generate diagram notes
     *
     * This method adds any notes in the diagram, which were declared in the
     * code using inline directives
     *
     * @param ostr Output stream
     * @param element Element to which the note should be attached
     */
    void generate_notes(
        std::ostream &ostr, const model::element &element) const;

    /**
     * @brief Generate diagram element PlantUML style
     *
     * This method renders a style for a specific `el` element if specified
     * in the config file or inline comment directive.
     *
     * @param ostr Output stream
     * @param element_type Name of the element type (e.g. "class")
     * @param el Reference to a stylable diagram element
     */
    void generate_style(std::ostream &ostr, const std::string &element_type,
        const model::stylable_element &el) const;

    /**
     * @brief Generate comment with diagram metadata
     *
     * @param ostr Output stream
     */
    void generate_metadata(std::ostream &ostr) const;

    /**
     * @brief Generate diagram title
     *
     * Generates a PlantUML diagram title directive if diagram title
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

private:
    void generate_row_column_hints(std::ostream &ostr,
        const std::string &entity_name, const config::layout_hint &hint) const;

    void generate_position_hints(std::ostream &ostr,
        const std::string &entity_name, const config::layout_hint &hint) const;

protected:
    mutable std::set<std::string> m_generated_aliases;
};

template <typename C, typename D>
void generator<C, D>::generate(std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();
    const auto &model = generators::generator<C, D>::model();

    generators::generator<C, D>::update_context();

    if (!config.allow_empty_diagrams() && model.is_empty() &&
        config.puml().before.empty() && config.puml().after.empty()) {
        throw clanguml::error::empty_diagram_error{model.type(), model.name(),
            "Diagram configuration resulted in empty diagram."};
    }

    ostr << "@startuml" << '\n';

    generate_title(ostr);

    // Generate PlantUML directives before auto generated content
    generate_plantuml_directives(ostr, config.puml().before);

    generate_diagram(ostr);

    // Generate PlantUML directives after auto generated content
    generate_plantuml_directives(ostr, config.puml().after);

    generate_metadata(ostr);

    ostr << "@enduml" << '\n';
}

template <typename C, typename D>
void generator<C, D>::generate_config_layout_hints(std::ostream &ostr) const
{
    using namespace clanguml::util;

    const auto &config = generators::generator<C, D>::config();

    // Generate layout hints
    for (const auto &[entity_name, hints] : config.layout()) {
        for (const auto &hint : hints) {
            try {
                if (hint.hint == config::hint_t::together) {
                    // 'together' layout hint is handled separately
                }
                else if (hint.hint == config::hint_t::row ||
                    hint.hint == config::hint_t::column) {
                    generate_row_column_hints(ostr, entity_name, hint);
                }
                else {
                    generate_position_hints(ostr, entity_name, hint);
                }
            }
            catch (clanguml::error::uml_alias_missing &e) {
                LOG_DBG("=== Skipping layout hint '{}' from {} due "
                        "to: {}",
                    to_string(hint.hint), entity_name, e.what());
            }
        }
    }
}

template <typename C, typename D>
void generator<C, D>::generate_row_column_hints(std::ostream &ostr,
    const std::string &entity_name, const config::layout_hint &hint) const
{
    const auto &config = generators::generator<C, D>::config();
    const auto &model = generators::generator<C, D>::model();

    const auto &uns = config.using_namespace();

    std::vector<std::string> group_elements;
    std::vector<std::pair<std::string, std::string>> element_aliases_pairs;

    group_elements.push_back(entity_name);
    const auto &group_tail = std::get<std::vector<std::string>>(hint.entity);
    std::copy(group_tail.begin(), group_tail.end(),
        std::back_inserter(group_elements));

    auto element_opt = model.get(entity_name);
    if (!element_opt)
        element_opt = model.get((uns | entity_name).to_string());

    for (auto it = cbegin(group_elements);
         it != cend(group_elements) && std::next(it) != cend(group_elements);
         ++it) {
        const auto &first = *it;
        const auto &second = *std::next(it);

        auto first_opt = model.get(first);
        if (!first_opt)
            first_opt = model.get((uns | first).to_string());

        auto second_opt = model.get(second);
        if (!second_opt)
            second_opt = model.get((uns | second).to_string());

        element_aliases_pairs.emplace_back(
            first_opt.value().alias(), second_opt.value().alias());
    }

    std::string hint_direction =
        hint.hint == clanguml::config::hint_t::row ? "right" : "down";

    for (const auto &[f, s] : element_aliases_pairs) {
        ostr << f << " -[hidden]" << hint_direction << "- " << s << '\n';
    }
}

template <typename C, typename D>
void generator<C, D>::generate_position_hints(std::ostream &ostr,
    const std::string &entity_name, const config::layout_hint &hint) const
{
    const auto &config = generators::generator<C, D>::config();
    const auto &model = generators::generator<C, D>::model();

    const auto &uns = config.using_namespace();

    const auto &hint_entity = std::get<std::string>(hint.entity);

    auto element_opt = model.get(entity_name);
    if (!element_opt)
        element_opt = model.get((uns | entity_name).to_string());

    auto hint_element_opt = model.get(hint_entity);
    if (!hint_element_opt)
        hint_element_opt = model.get((uns | hint_entity).to_string());

    if (!element_opt || !hint_element_opt)
        return;

    std::stringstream hint_str;

    hint_str << element_opt.value().alias() << " -[hidden]"
             << clanguml::config::to_string(hint.hint) << "- "
             << hint_element_opt.value().alias() << '\n';

    ostr << hint_str.str();
}

template <typename C, typename D>
void generator<C, D>::generate_plantuml_directives(
    std::ostream &ostr, const std::vector<std::string> &directives) const
{
    using common::model::namespace_;

    for (const auto &d : directives) {
        auto rendered_directive =
            common::jinja::render_template(generators::generator<C, D>::env(),
                generators::generator<C, D>::context(), d);

        if (rendered_directive)
            ostr << *rendered_directive << '\n';
    }
}

template <typename C, typename D>
void generator<C, D>::generate_style(std::ostream &ostr,
    const std::string &element_type, const model::stylable_element &el) const
{
    const auto &config = generators::generator<C, D>::config();

    if (el.style() && !el.style().value().empty()) // NOLINT
        ostr << " " << *el.style();                // NOLINT
    else if (config.puml) {
        if (const auto config_style = config.puml().get_style(element_type);
            config_style) {
            ostr << " " << *config_style;
        }
    }
}

template <typename C, typename D>
void generator<C, D>::generate_notes(
    std::ostream &ostr, const model::element &e) const
{
    const auto &config = generators::generator<C, D>::config();

    for (const auto &decorator : e.decorators()) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(config.name)) {
            ostr << "note " << note->position << " of " << e.alias() << '\n'
                 << note->text << '\n'
                 << "end note\n";
        }
    }
}

template <typename C, typename D>
void generator<C, D>::generate_metadata(std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();

    if (config.generate_metadata()) {
        ostr << '\n'
             << "'Generated with clang-uml, version "
             << clanguml::version::version() << '\n'
             << "'LLVM version " << clang::getClangFullVersion() << '\n';
    }
}

template <typename C, typename D>
void generator<C, D>::generate_title(std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();

    if (config.title) {
        ostr << "title " << config.title() << '\n';
    }
}

template <typename C, typename D>
template <typename E>
void generator<C, D>::generate_link(std::ostream &ostr, const E &e) const
{
    if (e.file().empty() && e.file_relative().empty())
        return;

    auto maybe_link_pattern = generators::generator<C, D>::get_link_pattern(e);

    if (!maybe_link_pattern) {
        return;
    }

    inja::json e_ctx =
        common::jinja::jinja_context<common::model::diagram_element>(e);

    const auto &[link_prefix, link_pattern] = *maybe_link_pattern;

    ostr << " [[";
    try {
        if (!link_pattern.empty()) {
            auto ec = generators::generator<C, D>::element_context(e, e_ctx);
            common::generators::make_context_source_relative(ec, link_prefix);
            ostr << generators::generator<C, D>::env().render(
                std::string_view{link_pattern}, ec);
        }
    }
    catch (const inja::json::parse_error &e) {
        LOG_ERROR("Failed to parse Jinja template: {}", link_pattern);
    }
    catch (const std::exception &e) {
        LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
            link_pattern, e.what());
    }

    auto maybe_tooltip_pattern =
        generators::generator<C, D>::get_tooltip_pattern(e);

    if (maybe_tooltip_pattern) {
        const auto &[tooltip_prefix, tooltip_pattern] = *maybe_tooltip_pattern;

        ostr << "{";
        try {
            auto ec = generators::generator<C, D>::element_context(e, e_ctx);
            common::generators::make_context_source_relative(
                ec, tooltip_prefix);
            if (!tooltip_pattern.empty()) {
                ostr << generators::generator<C, D>::env().render(
                    std::string_view{tooltip_pattern}, ec);
            }
        }
        catch (const inja::json::parse_error &e) {
            LOG_ERROR("Failed to parse Jinja template: {}", tooltip_pattern);
        }
        catch (const std::exception &e) {
            LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
                tooltip_pattern, e.what());
        }
        ostr << "}";
    }
    ostr << "]]";
}

template <typename C, typename D>
void generator<C, D>::print_debug(
    const common::model::source_location &e, std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();

    if (config.debug_mode())
        ostr << "' " << e.file() << ":" << e.line() << '\n';
}

template <typename DiagramModel, typename DiagramConfig>
std::ostream &operator<<(
    std::ostream &os, const generator<DiagramModel, DiagramConfig> &g)
{
    g.generate(os);
    return os;
}

} // namespace clanguml::common::generators::plantuml