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

#include "config/config.h"
#include "util/error.h"
#include "util/util.h"

#include <cppast/libclang_parser.hpp>
#include <glob/glob.hpp>

#include <ostream>

namespace clanguml::common::generators::plantuml {

using clanguml::common::model::message_t;
using clanguml::common::model::relationship_t;
using clanguml::common::model::scope_t;

std::string to_plantuml(relationship_t r, std::string style);
std::string to_plantuml(scope_t scope);
std::string to_plantuml(message_t r);

template <typename ConfigType, typename DiagramType> class generator {
public:
    generator(ConfigType &config, DiagramType &model)
        : m_config{config}
        , m_model{model}
    {
    }

    virtual ~generator() = default;

    virtual void generate(std::ostream &ostr) const = 0;

    template <typename C, typename D>
    friend std::ostream &operator<<(std::ostream &os, const generator<C, D> &g);

    void generate_config_layout_hints(std::ostream &ostr) const;

    void generate_plantuml_directives(
        std::ostream &ostr, const std::vector<std::string> &directives) const;

    void generate_notes(
        std::ostream &ostr, const model::element &decorators) const;

protected:
    ConfigType &m_config;
    DiagramType &m_model;
};

template <typename C, typename D>
std::ostream &operator<<(std::ostream &os, const generator<C, D> &g)
{
    g.generate(os);
    return os;
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
                LOG_ERROR("=== Skipping layout hint from {} to {} due "
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

    for (const auto &b : directives) {
        std::string note{b};
        std::tuple<std::string, size_t, size_t> alias_match;
        while (util::find_element_alias(note, alias_match)) {
            auto alias = m_model.to_alias(
                m_config.using_namespace().relative(std::get<0>(alias_match)));
            note.replace(
                std::get<1>(alias_match), std::get<2>(alias_match), alias);
        }
        ostr << note << '\n';
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

template <typename DiagramModel, typename DiagramConfig,
    typename DiagramVisitor>
DiagramModel generate(cppast::libclang_compilation_database &db,
    const std::string &name, DiagramConfig &diagram)
{
    LOG_INFO("Generating diagram {}.puml", name);
    DiagramModel d;
    d.set_name(name);

    // Get all translation units matching the glob from diagram
    // configuration
    std::vector<std::string> translation_units{};
    for (const auto &g : diagram.glob()) {
        LOG_DBG("Processing glob: {}", g);
        const auto matches = glob::rglob(g);
        std::copy(matches.begin(), matches.end(),
            std::back_inserter(translation_units));
    }

    cppast::cpp_entity_index idx;
    cppast::simple_file_parser<cppast::libclang_parser> parser{
        type_safe::ref(idx)};

    // Process all matching translation units
    DiagramVisitor ctx(idx, d, diagram);
    cppast::parse_files(parser, translation_units, db);
    for (auto &file : parser.files())
        ctx(file);

    return std::move(d);
}

}
