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

#include <cppast/libclang_parser.hpp>
#include <glob/glob.hpp>
#include <inja/inja.hpp>

#include <ostream>

namespace clanguml::common::generators::plantuml {

using clanguml::common::model::access_t;
using clanguml::common::model::element;
using clanguml::common::model::message_t;
using clanguml::common::model::relationship_t;

std::string to_plantuml(relationship_t r, std::string style);
std::string to_plantuml(access_t scope);
std::string to_plantuml(message_t r);

template <typename ConfigType, typename DiagramType> class generator {
public:
    generator(ConfigType &config, DiagramType &model)
        : m_config{config}
        , m_model{model}
    {
        init_context();
        init_env();
    }

    virtual ~generator() = default;

    virtual void generate(std::ostream &ostr) const = 0;

    template <typename C, typename D>
    friend std::ostream &operator<<(std::ostream &os, const generator<C, D> &g);

    void generate_config_layout_hints(std::ostream &ostr) const;

    void generate_plantuml_directives(
        std::ostream &ostr, const std::vector<std::string> &directives) const;

    void generate_notes(
        std::ostream &ostr, const model::element &element) const;

    template <typename E>
    void generate_link(std::ostream &ostr, const E &e) const;

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
    inja::json m_context;
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
        std::string c = e.comment().value();
        if (!c.empty()) {
            ctx["element"]["comment"] = util::trim(c);
        }
    }

    return ctx;
}

template <typename C, typename D>
void generator<C, D>::generate_config_layout_hints(std::ostream &ostr) const
{
    using namespace clanguml::util;

    const auto &uns = m_config.using_namespace();

    // Generate layout hints
    for (const auto &[entity, hints] : m_config.layout()) {
        for (const auto &hint : hints) {
            std::stringstream hint_str;
            try {
                hint_str << m_model.to_alias(uns.relative(entity))
                         << " -[hidden]"
                         << clanguml::config::to_string(hint.hint) << "- "
                         << m_model.to_alias(uns.relative(hint.entity)) << '\n';
                ostr << hint_str.str();
            }
            catch (clanguml::error::uml_alias_missing &e) {
                LOG_DBG("=== Skipping layout hint from {} to {} due "
                        "to: {}",
                    entity, hint.entity, e.what());
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
        // Render the directive with template engine first
        std::string directive{env().render(std::string_view{d}, context())};

        // Now search for alias @A() directives in the text
        std::tuple<std::string, size_t, size_t> alias_match;
        while (util::find_element_alias(directive, alias_match)) {
            auto alias = m_model.to_alias(
                m_config.using_namespace().relative(std::get<0>(alias_match)));
            directive.replace(
                std::get<1>(alias_match), std::get<2>(alias_match), alias);
        }
        ostr << directive << '\n';
    }
}

template <typename C, typename D>
void generator<C, D>::generate_notes(
    std::ostream &ostr, const model::element &e) const
{
    for (auto decorator : e.decorators()) {
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

    if (!m_config.generate_links().link.empty()) {
        ostr << " [[";
        ostr << env().render(std::string_view{m_config.generate_links().link},
            element_context(e));
    }

    if (!m_config.generate_links().tooltip.empty()) {
        ostr << "{";
        ostr << env().render(
            std::string_view{m_config.generate_links().tooltip},
            element_context(e));
        ostr << "}";
    }

    ostr << "]]";
}

template <typename DiagramModel, typename DiagramConfig,
    typename DiagramVisitor>
std::unique_ptr<DiagramModel> generate(
    const cppast::libclang_compilation_database &db, const std::string &name,
    DiagramConfig &config, bool verbose = false)
{
    LOG_INFO("Generating diagram {}.puml", name);
    auto diagram = std::make_unique<DiagramModel>();
    diagram->set_name(name);
    diagram->set_filter(
        std::make_unique<model::diagram_filter>(*diagram, config));

    // Get all translation units matching the glob from diagram
    // configuration
    std::vector<std::string> translation_units{};
    for (const auto &g : config.glob()) {
        LOG_DBG("Processing glob: {}", g);
        const auto matches = glob::rglob(g);
        std::copy(matches.begin(), matches.end(),
            std::back_inserter(translation_units));
    }

    cppast::cpp_entity_index idx;
    auto logger =
        verbose ? cppast::default_logger() : cppast::default_quiet_logger();
    cppast::simple_file_parser<cppast::libclang_parser> parser{
        type_safe::ref(idx), std::move(logger)};

    // Process all matching translation units
    DiagramVisitor ctx(idx, *diagram, config);
    cppast::parse_files(parser, translation_units, db);
    for (auto &file : parser.files())
        ctx(file);

    diagram->set_complete(true);

    return std::move(diagram);
}

template <typename C, typename D> void generator<C, D>::init_context()
{
    if (m_config.git) {
        m_context["git"]["branch"] = m_config.git().branch;
        m_context["git"]["revision"] = m_config.git().revision;
        m_context["git"]["commit"] = m_config.git().commit;
        m_context["git"]["toplevel"] = m_config.git().toplevel;
    }

    m_context["diagram"]["name"] = m_config.name;
    m_context["diagram"]["type"] = to_string(m_config.type());
}

template <typename C, typename D> void generator<C, D>::init_env()
{
    //
    // Add basic string functions to inja environment
    //
    m_env.add_callback("empty", 1, [](inja::Arguments &args) {
        return args.at(0)->get<std::string>().empty();
    });

    m_env.add_callback("ltrim", 1, [](inja::Arguments &args) {
        return util::ltrim(args.at(0)->get<std::string>());
    });

    m_env.add_callback("rtrim", 1, [](inja::Arguments &args) {
        return util::rtrim(args.at(0)->get<std::string>());
    });

    m_env.add_callback("trim", 1, [](inja::Arguments &args) {
        return util::trim(args.at(0)->get<std::string>());
    });

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

    // Convert C++ entity to PlantUML alias, e.g.
    //   "note left of {{ alias("ClassA") }}: This is a note"
    // is equivalent to the old syntax:
    //   "note left of @A(ClassA): This is a note"
    m_env.add_callback("alias", 1, [this](inja::Arguments &args) {
        auto alias_match = args[0]->get<std::string>();
        return m_model.to_alias(
            m_config.using_namespace().relative(alias_match));
    });

    m_env.add_callback("comment", 1, [this](inja::Arguments &args) {
        std::string res{};
        auto full_name = args[0]->get<std::string>();
        auto element = m_model.get(full_name);

        if (!element.has_value()) {
            // Try with current using namespace prepended
            element = m_model.get(fmt::format(
                "{}::{}", m_config.using_namespace().to_string(), full_name));
        }

        if (element.has_value()) {
            auto comment = element.value().comment();

            if (comment.has_value())
                res = comment.value();
        }

        return res;
    });
}

}
