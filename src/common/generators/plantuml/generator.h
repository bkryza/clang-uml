/**
 * src/common/generators/plantuml/generator.h
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

#include "common/model/diagram_filter.h"
#include "config/config.h"
#include "util/error.h"
#include "util/util.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <glob/glob.hpp>
#include <inja/inja.hpp>

#include <ostream>

namespace clanguml::common::generators::plantuml {

using clanguml::common::model::access_t;
using clanguml::common::model::element;
using clanguml::common::model::message_t;
using clanguml::common::model::relationship_t;

std::string to_plantuml(relationship_t r, const std::string &style);
std::string to_plantuml(access_t scope);
std::string to_plantuml(message_t r);

/**
 * @brief Base class for diagram generators
 *
 * @tparam ConfigType Configuration type
 * @tparam DiagramType Diagram model type
 */
template <typename ConfigType, typename DiagramType> class generator {
public:
    /**
     * @brief Constructor
     *
     * @param config Reference to instance of @link clanguml::config::diagram
     * @param model Reference to instance of @link clanguml::model::diagram
     */
    generator(ConfigType &config, DiagramType &model)
        : m_config{config}
        , m_model{model}
    {
        init_context();
        init_env();
    }

    virtual ~generator() = default;

    /**
     * @brief Generate diagram
     *
     * This method must be implemented in subclasses for specific diagram
     * types. It is responsible for calling other methods in appropriate
     * order to generate the diagram into the output stream.
     *
     * @param ostr Output stream
     */
    virtual void generate(std::ostream &ostr) const = 0;

    template <typename C, typename D>
    friend std::ostream &operator<<(std::ostream &os, const generator<C, D> &g);

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
    ConfigType &m_config;
    DiagramType &m_model;
    mutable std::set<std::string> m_generated_aliases;
    mutable inja::json m_context;
    mutable inja::Environment m_env;
};

template <typename C, typename D>
std::ostream &operator<<(std::ostream &os, const generator<C, D> &g)
{
    g.generate(os);
    return os;
}

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
        std::string relative_path = file.string();

        if (file.is_absolute() && ctx.template contains("git"))
            relative_path =
                std::filesystem::relative(file, ctx["git"]["toplevel"]);

        ctx["element"]["source"]["path"] = relative_path;
        ctx["element"]["source"]["full_path"] = file.string();
        ctx["element"]["source"]["name"] = file.filename();
        ctx["element"]["source"]["line"] = e.line();
    }

    if (e.comment().has_value()) {
        ctx["element"]["comment"] = e.comment().value();
    }

    return ctx;
}

template <typename C, typename D>
void generator<C, D>::generate_config_layout_hints(std::ostream &ostr) const
{
    using namespace clanguml::util;

    const auto &uns = m_config.using_namespace();

    // Generate layout hints
    for (const auto &[entity_name, hints] : m_config.layout()) {
        for (const auto &hint : hints) {
            std::stringstream hint_str;
            try {
                auto element_opt = m_model.get(entity_name);
                if (!element_opt)
                    element_opt = m_model.get((uns | entity_name).to_string());

                auto hint_element_opt = m_model.get(hint.entity);
                if (!hint_element_opt)
                    hint_element_opt =
                        m_model.get((uns | hint.entity).to_string());

                if (!element_opt || !hint_element_opt)
                    continue;
                hint_str << element_opt.value().alias() << " -[hidden]"
                         << clanguml::config::to_string(hint.hint) << "- "
                         << hint_element_opt.value().alias() << '\n';
                ostr << hint_str.str();
            }
            catch (clanguml::error::uml_alias_missing &e) {
                LOG_DBG("=== Skipping layout hint from {} to {} due "
                        "to: {}",
                    entity_name, hint.entity, e.what());
            }
        }
    }
}

template <typename C, typename D>
void generator<C, D>::generate_plantuml_directives(
    std::ostream &ostr, const std::vector<std::string> &directives) const
{
    using common::model::namespace_;

    for (const auto &d : directives) {
        try {
            // Render the directive with template engine first
            std::string directive{env().render(std::string_view{d}, context())};

            // Now search for alias @A() directives in the text
            // (this is deprecated)
            std::tuple<std::string, size_t, size_t> alias_match;
            while (util::find_element_alias(directive, alias_match)) {
                const auto full_name =
                    m_config.using_namespace() | std::get<0>(alias_match);
                auto element_opt = m_model.get(full_name.to_string());

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
        catch (const inja::json::parse_error &e) {
            LOG_ERROR("Failed to parse Jinja template: {}", d);
        }
        catch (const inja::json::exception &e) {
            LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
                d, e.what());
        }
    }
}

template <typename C, typename D>
void generator<C, D>::generate_notes(
    std::ostream &ostr, const model::element &e) const
{
    for (const auto &decorator : e.decorators()) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(m_config.name)) {
            ostr << "note " << note->position << " of " << e.alias() << '\n'
                 << note->text << '\n'
                 << "end note\n";
        }
    }
}

template <typename C, typename D>
template <typename E>
void generator<C, D>::generate_link(std::ostream &ostr, const E &e) const
{
    if (e.file().empty())
        return;

    ostr << " [[";
    try {
        if (!m_config.generate_links().link.empty()) {

            ostr << env().render(
                std::string_view{m_config.generate_links().link},
                element_context(e));
        }
    }
    catch (const inja::json::parse_error &e) {
        LOG_ERROR("Failed to parse Jinja template: {}",
            m_config.generate_links().link);
    }
    catch (const inja::json::exception &e) {
        LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
            m_config.generate_links().link, e.what());
    }

    ostr << "{";
    try {
        if (!m_config.generate_links().tooltip.empty()) {
            ostr << env().render(
                std::string_view{m_config.generate_links().tooltip},
                element_context(e));
        }
    }
    catch (const inja::json::parse_error &e) {
        LOG_ERROR("Failed to parse Jinja template: {}",
            m_config.generate_links().link);
    }
    catch (const inja::json::exception &e) {
        LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
            m_config.generate_links().link, e.what());
    }

    ostr << "}";
    ostr << "]]";
}

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

template <typename C, typename D> void generator<C, D>::init_context()
{
    if (m_config.git) {
        m_context["git"]["branch"] = m_config.git().branch;
        m_context["git"]["revision"] = m_config.git().revision;
        m_context["git"]["commit"] = m_config.git().commit;
        m_context["git"]["toplevel"] = m_config.git().toplevel;
    }
}

template <typename C, typename D> void generator<C, D>::update_context() const
{
    m_context["diagram"] = m_model.context();
}

template <typename C, typename D> void generator<C, D>::init_env()
{
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
    m_env.add_callback("element", 1, [this](inja::Arguments &args) {
        inja::json res{};
        auto element_opt = m_model.get_with_namespace(
            args[0]->get<std::string>(), m_config.using_namespace());

        if (element_opt.has_value())
            res = element_opt.value().context();

        return res;
    });

    // Convert C++ entity to PlantUML alias, e.g.
    //   "note left of {{ alias("A") }}: This is a note"
    // Shortcut to:
    //   {{ element("A").alias }}
    //
    m_env.add_callback("alias", 1, [this](inja::Arguments &args) {
        auto element_opt = m_model.get_with_namespace(
            args[0]->get<std::string>(), m_config.using_namespace());

        return element_opt.value().alias();
    });

    // Get elements' comment:
    //   "note left of {{ alias("A") }}: {{ comment("A") }}"
    // Shortcut to:
    //   {{ element("A").comment }}
    //
    m_env.add_callback("comment", 1, [this](inja::Arguments &args) {
        inja::json res{};
        auto element = m_model.get_with_namespace(
            args[0]->get<std::string>(), m_config.using_namespace());

        if (element.has_value()) {
            auto comment = element.value().comment();

            if (comment.has_value()) {
                assert(comment.value().is_object());
                res = comment.value();
            }
        }

        return res;
    });
}
} // namespace clanguml::common::generators::plantuml