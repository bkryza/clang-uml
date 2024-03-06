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
#include "common/model/diagram_filter.h"
#include "common/model/relationship.h"
#include "config/config.h"
#include "util/error.h"
#include "util/util.h"
#include "version.h"

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
        init_context();
        init_env();
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
    /**
     * @brief Update diagram Jinja context
     *
     * This method updates the diagram context with models properties
     * which can be used to render Jinja templates in the diagram (e.g.
     * in notes or links)
     */
    void update_context() const;

protected:
    const inja::json &context() const;

    inja::Environment &env() const;

    template <typename E> inja::json element_context(const E &e) const;

private:
    void generate_row_column_hints(std::ostream &ostr,
        const std::string &entity_name, const config::layout_hint &hint) const;

    void generate_position_hints(std::ostream &ostr,
        const std::string &entity_name, const config::layout_hint &hint) const;

    void init_context();

    void init_env();

protected:
    mutable std::set<std::string> m_generated_aliases;
    mutable inja::json m_context;
    mutable inja::Environment m_env;
};

template <typename C, typename D>
const inja::json &generator<C, D>::context() const
{
    return m_context;
}

template <typename C, typename D>
inja::Environment &generator<C, D>::env() const
{
    return m_env;
}

template <typename C, typename D>
template <typename E>
inja::json generator<C, D>::element_context(const E &e) const
{
    auto ctx = context();

    ctx["element"] = e.context();

    if (!e.file().empty()) {
        std::filesystem::path file{e.file()};
        std::string git_relative_path = file.string();
        if (!e.file_relative().empty()) {
#if _MSC_VER
            if (file.is_absolute() && ctx.contains("git"))
#else
            if (file.is_absolute() && ctx.template contains("git"))
#endif
                git_relative_path =
                    std::filesystem::relative(file, ctx["git"]["toplevel"])
                        .string();
        }
        else {
            git_relative_path = "";
        }

        ctx["element"]["source"]["path"] = util::path_to_url(git_relative_path);
        ctx["element"]["source"]["full_path"] = file.string();
        ctx["element"]["source"]["name"] = file.filename().string();
        ctx["element"]["source"]["line"] = e.line();
    }

    const auto &maybe_comment = e.comment();
    if (maybe_comment) {
        ctx["element"]["comment"] = maybe_comment.value();
    }

    return ctx;
}

template <typename C, typename D>
void generator<C, D>::generate(std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();
    const auto &model = generators::generator<C, D>::model();

    update_context();

    if (!config.allow_empty_diagrams() && model.is_empty() &&
        config.puml().before.empty() && config.puml().after.empty()) {
        throw clanguml::error::empty_diagram_error{
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
    const auto &config = generators::generator<C, D>::config();
    const auto &model = generators::generator<C, D>::model();

    using common::model::namespace_;

    for (const auto &d : directives) {
        try {
            // Render the directive with template engine first
            std::string directive{env().render(std::string_view{d}, context())};

            // Now search for alias `@A()` directives in the text
            // (this is deprecated)
            std::tuple<std::string, size_t, size_t> alias_match;
            while (util::find_element_alias(directive, alias_match)) {
                const auto full_name =
                    config.using_namespace() | std::get<0>(alias_match);
                auto element_opt = model.get(full_name.to_string());

                if (element_opt)
                    directive.replace(std::get<1>(alias_match),
                        std::get<2>(alias_match), element_opt.value().alias());
                else {
                    LOG_ERROR("Cannot find clang-uml alias for element {}",
                        full_name.to_string());
                    directive.replace(std::get<1>(alias_match),
                        std::get<2>(alias_match), "UNKNOWN_ALIAS");
                }
            }

            ostr << directive << '\n';
        }
        catch (const clanguml::error::uml_alias_missing &e) {
            LOG_ERROR("Failed to render PlantUML directive due to unresolvable "
                      "alias: {}",
                e.what());
        }
        catch (const inja::json::parse_error &e) {
            LOG_ERROR("Failed to parse Jinja template: {}", d);
        }
        catch (const inja::json::exception &e) {
            LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
                d, e.what());
        }
        catch (const std::regex_error &e) {
            LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to "
                      "std::regex_error: {}",
                d, e.what());
        }
        catch (const std::exception &e) {
            LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
                d, e.what());
        }
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
             << clanguml::version::CLANG_UML_VERSION << '\n'
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
    const auto &config = generators::generator<C, D>::config();

    if (e.file_relative().empty())
        return;

    ostr << " [[";
    try {
        if (!config.generate_links().link.empty()) {
            ostr << env().render(std::string_view{config.generate_links().link},
                element_context(e));
        }
    }
    catch (const inja::json::parse_error &e) {
        LOG_ERROR(
            "Failed to parse Jinja template: {}", config.generate_links().link);
    }
    catch (const inja::json::exception &e) {
        LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
            config.generate_links().link, e.what());
    }

    ostr << "{";
    try {
        if (!config.generate_links().tooltip.empty()) {
            ostr << env().render(
                std::string_view{config.generate_links().tooltip},
                element_context(e));
        }
    }
    catch (const inja::json::parse_error &e) {
        LOG_ERROR(
            "Failed to parse Jinja template: {}", config.generate_links().link);
    }
    catch (const inja::json::exception &e) {
        LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
            config.generate_links().link, e.what());
    }

    ostr << "}";
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

template <typename C, typename D> void generator<C, D>::init_context()
{
    const auto &config = generators::generator<C, D>::config();

    if (config.git) {
        m_context["git"]["branch"] = config.git().branch;
        m_context["git"]["revision"] = config.git().revision;
        m_context["git"]["commit"] = config.git().commit;
        m_context["git"]["toplevel"] = config.git().toplevel;
    }
}

template <typename C, typename D> void generator<C, D>::update_context() const
{
    m_context["diagram"] = generators::generator<C, D>::model().context();
}

template <typename C, typename D> void generator<C, D>::init_env()
{
    const auto &model = generators::generator<C, D>::model();
    const auto &config = generators::generator<C, D>::config();

    //
    // Add basic string functions to inja environment
    //

    // Check if string is empty
    m_env.add_callback("empty", 1, [](inja::Arguments &args) {
        return args.at(0)->get<std::string>().empty();
    });

    // Remove spaces from the left of a string
    m_env.add_callback("ltrim", 1, [](inja::Arguments &args) {
        return util::ltrim(args.at(0)->get<std::string>());
    });

    // Remove trailing spaces from a string
    m_env.add_callback("rtrim", 1, [](inja::Arguments &args) {
        return util::rtrim(args.at(0)->get<std::string>());
    });

    // Remove spaces before and after a string
    m_env.add_callback("trim", 1, [](inja::Arguments &args) {
        return util::trim(args.at(0)->get<std::string>());
    });

    // Make a string shorted with a limit to
    m_env.add_callback("abbrv", 2, [](inja::Arguments &args) {
        return util::abbreviate(
            args.at(0)->get<std::string>(), args.at(1)->get<unsigned>());
    });

    m_env.add_callback("replace", 3, [](inja::Arguments &args) {
        std::string result = args[0]->get<std::string>();
        std::regex pattern(args[1]->get<std::string>());
        return std::regex_replace(result, pattern, args[2]->get<std::string>());
    });

    m_env.add_callback("split", 2, [](inja::Arguments &args) {
        return util::split(
            args[0]->get<std::string>(), args[1]->get<std::string>());
    });

    //
    // Add PlantUML specific functions
    //

    // Return the entire element JSON context based on element name
    // e.g.:
    //   {{ element("clanguml::t00050::A").comment }}
    //
    m_env.add_callback("element", 1, [&model, &config](inja::Arguments &args) {
        inja::json res{};
        auto element_opt = model.get_with_namespace(
            args[0]->get<std::string>(), config.using_namespace());

        if (element_opt.has_value())
            res = element_opt.value().context();

        return res;
    });

    // Convert C++ entity to PlantUML alias, e.g.
    //   "note left of {{ alias("A") }}: This is a note"
    // Shortcut to:
    //   {{ element("A").alias }}
    //
    m_env.add_callback("alias", 1, [&model, &config](inja::Arguments &args) {
        auto element_opt = model.get_with_namespace(
            args[0]->get<std::string>(), config.using_namespace());

        if (!element_opt.has_value())
            throw clanguml::error::uml_alias_missing(
                args[0]->get<std::string>());

        return element_opt.value().alias();
    });

    // Get elements' comment:
    //   "note left of {{ alias("A") }}: {{ comment("A") }}"
    // Shortcut to:
    //   {{ element("A").comment }}
    //
    m_env.add_callback("comment", 1, [&model, &config](inja::Arguments &args) {
        inja::json res{};
        auto element_opt = model.get_with_namespace(
            args[0]->get<std::string>(), config.using_namespace());

        if (!element_opt.has_value())
            throw clanguml::error::uml_alias_missing(
                args[0]->get<std::string>());

        auto comment = element_opt.value().comment();

        if (comment.has_value()) {
            assert(comment.value().is_object());
            res = comment.value();
        }

        return res;
    });
}
} // namespace clanguml::common::generators::plantuml