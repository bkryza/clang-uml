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
#include "common/model/diagram_filter.h"
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

namespace clanguml::common::generators::mermaid {

using clanguml::common::model::access_t;
using clanguml::common::model::element;
using clanguml::common::model::message_t;
using clanguml::common::model::relationship_t;

std::string to_mermaid(relationship_t r);
std::string to_mermaid(access_t scope);
std::string to_mermaid(message_t r);

std::string indent(unsigned level);

std::string render_name(std::string name, bool round_brackets = true);

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

    const auto maybe_comment = e.comment();
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

    if (!config.allow_empty_diagrams() && model.is_empty() &&
        config.mermaid().before.empty() && config.mermaid().after.empty()) {
        throw clanguml::error::empty_diagram_error{
            "Diagram configuration resulted in empty diagram."};
    }

    update_context();

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
    const auto &config = generators::generator<C, D>::config();

    if (e.file().empty())
        return;

    if (config.generate_links().link.empty() &&
        config.generate_links().tooltip.empty())
        return;

    ostr << indent(1) << "click " << e.alias() << " href \"";
    try {
        std::string link{};
        if (!config.generate_links().link.empty()) {
            link = env().render(std::string_view{config.generate_links().link},
                element_context(e));
        }
        if (link.empty())
            link = " ";
        ostr << link;
    }
    catch (const inja::json::parse_error &e) {
        LOG_ERROR(
            "Failed to parse Jinja template: {}", config.generate_links().link);
        ostr << " ";
    }
    catch (const inja::json::exception &e) {
        LOG_ERROR("Failed to render comment directive: \n{}\n due to: {}",
            config.generate_links().link, e.what());
        ostr << " ";
    }
    ostr << "\"";

    if (!config.generate_links().tooltip.empty()) {
        ostr << " \"";
        try {
            auto tooltip_text =
                env().render(std::string_view{config.generate_links().tooltip},
                    element_context(e));
            util::replace_all(tooltip_text, "\"", "&bdquo;");
            ostr << tooltip_text;
        }
        catch (const inja::json::parse_error &e) {
            LOG_ERROR("Failed to parse Jinja template: {}",
                config.generate_links().link);
            ostr << " ";
        }
        catch (const inja::json::exception &e) {
            LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
                config.generate_links().link, e.what());
            ostr << " ";
        }

        ostr << "\"";
    }
    ostr << "\n";
}

template <typename C, typename D>
void generator<C, D>::generate_mermaid_directives(
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

            ostr << indent(1) << directive << '\n';
        }
        catch (const clanguml::error::uml_alias_missing &e) {
            LOG_ERROR(
                "Failed to render MermaidJS directive due to unresolvable "
                "alias: {}",
                e.what());
        }
        catch (const inja::json::parse_error &e) {
            LOG_ERROR("Failed to parse Jinja template: {}", d);
        }
        catch (const inja::json::exception &e) {
            LOG_ERROR("Failed to render MermaidJS directive: \n{}\n due to: {}",
                d, e.what());
        }
        catch (const std::regex_error &e) {
            LOG_ERROR("Failed to render MermaidJS directive: \n{}\n due to "
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
             << clanguml::version::CLANG_UML_VERSION << '\n'
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
    // Add MermaidJS specific functions
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

    // Convert C++ entity to MermaidJS alias, e.g.
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
} // namespace clanguml::common::generators::mermaid