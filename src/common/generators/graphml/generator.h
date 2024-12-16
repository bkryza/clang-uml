/**
 * @file src/common/generators/graphml/generator.h
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
#include "common/model/element_view.h"
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
#include <pugixml/pugixml.hpp>

#include <ostream>

namespace clanguml::common::generators::graphml {

using clanguml::common::model::access_t;
using clanguml::common::model::element;
using clanguml::common::model::message_t;
using clanguml::common::model::relationship_t;

using graphml_t = pugi::xml_document;
using graphml_node_t = pugi::xml_node;

std::string render_name(std::string name);

/**
 * The types of graph nodes in the GraphML XML document
 */
enum class xml_node_t { kGraph, kNode, kEdge };

/**
 * Types of data node attributes that can be added to GraphML nodes
 * (xs:NMTOKEN)
 */
enum class property_type { kBoolean, kInt, kLong, kFloat, kDouble, kString };

std::string to_string(property_type t);

using key_property_map_t = std::map</*property name*/ std::string,
    std::pair</*property id*/ std::string, /*property type*/ property_type>>;

/**
 * Mapping of GraphML node properties and their keys. Necessary to generate
 * the <key> property definition elements at the beginning of GraphML document.
 */
class property_keymap_t {
public:
    property_keymap_t(std::string prefix);

    [[maybe_unused]] std::pair<std::string, property_type> add(
        const std::string &name, property_type pt = property_type::kString);

    std::optional<std::pair<std::string, property_type>> get(
        const std::string &name) const;

private:
    uint64_t next_data_key_id_{0};

    std::string prefix_;
    key_property_map_t map_;
};

/**
 * Mapping of diagram elements to their GraphML node ids. Each type of node
 * has a different prefix. With parser attribute set to `canonical`, the nodes
 * must have a `n` prefix and edges must have a `e` prefix.
 */
class graphml_node_map_t {
public:
    graphml_node_map_t(std::string prefix);

    [[maybe_unused]] std::string add(const std::string &name);

    std::optional<std::string> get(const std::string &name) const;

private:
    uint64_t next_node_id_{0};

    std::string prefix_;
    std::map<std::string, std::string> map_;
};

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
    using clanguml::common::generators::generator<ConfigType,
        DiagramType>::generator;
    ~generator() override = default;

    virtual std::vector<std::pair<std::string, property_type>>
    graph_property_names() const
    {
        using namespace std::string_literals;
        return {
            {"id"s, property_type::kString},
            {"diagram_type"s, property_type::kString},
            {"name"s, property_type::kString},
            {"using_namespace"s, property_type::kString},
        };
    }

    virtual std::vector<std::pair<std::string, property_type>>
    node_property_names() const
    {
        using namespace std::string_literals;
        return {{"id"s, property_type::kString},
            {"type"s, property_type::kString},
            {"name"s, property_type::kString},
            {"stereotype"s, property_type::kString},
            {"url"s, property_type::kString},
            {"tooltip"s, property_type::kString}};
    }

    virtual std::vector<std::pair<std::string, property_type>>
    edge_property_names() const
    {
        using namespace std::string_literals;
        return {{"type"s, property_type::kString},
            {"access"s, property_type::kString},
            {"label"s, property_type::kString},
            {"url"s, property_type::kString}};
    }

    /**
     * @brief Generate diagram
     *
     * This is the main diagram generation entrypoint. It is responsible for
     * calling other methods in appropriate order to generate the diagram
     * into the output stream. It generates diagram elements, that are
     * common to all types of diagrams in a given generator.
     *
     * @param ostr Output stream
     */
    void generate(std::ostream &ostr) const override;

    /**
     * @brief Generate diagram model
     *
     * This method must be implemented in subclasses for specific diagram
     * types.
     *
     * @param ostr Output stream
     */
    void generate_diagram(graphml_node_t &parent) const
    {
        generate_top_level_elements(parent);

        generate_notes(clanguml::common::generators::generator<ConfigType,
                           DiagramType>::model(),
            parent);

        generate_relationships(parent);
    }

    /**
     * @brief In a nested diagram, generate the top level elements.
     *
     * This method iterates over the top level elements. In case the diagram
     * is nested (i.e. includes packages), for each package it recursively
     * call generation of elements contained in each package.
     *
     * @param parent GraphML node
     */
    virtual void generate_top_level_elements(graphml_node_t &parent) const = 0;

    /**
     * @brief Generate any notes to be attached to diagram elements
     *
     * @param e Diagram element
     * @param parent Parent GraphML package node to attach the note to
     */
    template <typename T>
    void generate_notes(const T &e, graphml_node_t &parent) const;

    /**
     * @brief Generate metadata element with diagram metadata
     *
     * @param parent Root JSON object
     */
    void generate_metadata(graphml_t &parent) const;

    /**
     * @brief Generate diagram package
     *
     * @param p Diagram package element
     * @param parent Parent JSON node
     */
    virtual void generate(const model::package &p, graphml_node_t &parent) const
    {
    }

    /**
     * @brief Generate all relationships in the diagram.
     *
     * @param parent GraphML node
     */
    virtual void generate_relationships(graphml_node_t &parent) const;

    /**
     * @brief Generate all relationships originating at a diagram element.
     *
     * @tparam T Type of diagram element
     * @param c Diagram diagram element
     * @param parent JSON node
     */
    virtual void generate_relationships(
        const model::diagram_element &c, graphml_node_t &parent) const;

    template <typename T> void add_url(pugi::xml_node &node, const T &c) const;

    const property_keymap_t &graph_properties() const
    {
        return graph_properties_;
    }

    const property_keymap_t &node_properties() const
    {
        return node_properties_;
    }

    const property_keymap_t &edge_properties() const
    {
        return edge_properties_;
    }

    pugi::xml_node make_node(
        graphml_node_t &parent, const std::string &id) const;

    pugi::xml_node make_graph(
        graphml_node_t &parent, const std::string &id) const;

    pugi::xml_node make_subgraph(graphml_node_t &parent, const std::string &id,
        const std::string &name = "", const std::string &type = "") const;

    void add_data(pugi::xml_node &node, const std::string &key,
        const std::string &value, bool cdata = false) const;

    void add_cdata(pugi::xml_node &node, const std::string &key,
        const std::string &value) const;

protected:
    void generate_key(pugi::xml_node &parent, const std::string &attr_name,
        const std::string &for_value, const std::string &id_value,
        const std::string &attr_type = "string") const;

    property_keymap_t &graph_properties() { return graph_properties_; }
    property_keymap_t &node_properties() { return node_properties_; }
    property_keymap_t &edge_properties() { return edge_properties_; }

    mutable graphml_node_map_t graph_ids_{"g"};
    mutable graphml_node_map_t node_ids_{"n"};
    mutable graphml_node_map_t edge_ids_{"e"};

private:
    void generate_keys(graphml_node_t &parent) const;

    mutable property_keymap_t graph_properties_{"gd"};
    mutable property_keymap_t node_properties_{"nd"};
    mutable property_keymap_t edge_properties_{"ed"};

    mutable uint64_t edge_id_{0};
    mutable uint64_t note_id_{0};
};

template <typename DiagramModel, typename DiagramConfig>
std::ostream &operator<<(
    std::ostream &os, const generator<DiagramModel, DiagramConfig> &g)
{
    g.generate(os);
    return os;
}

template <typename C, typename D>
void generator<C, D>::generate(std::ostream &ostr) const
{
    const auto &config = generators::generator<C, D>::config();

    graphml_t graph;

    auto node_decl = graph.append_child(pugi::node_declaration);
    node_decl.append_attribute("version") = "1.0";
    node_decl.append_attribute("encoding") = "UTF-8";

    generate_metadata(graph);

    auto graphml = graph.append_child("graphml");
    graphml.append_attribute("xmlns") = "http://graphml.graphdrawing.org/xmlns";
    graphml.append_attribute("xmlns:xsi") =
        "http://www.w3.org/2001/XMLSchema-instance";
    graphml.append_attribute("xsi:schemaLocation") =
        "http://graphml.graphdrawing.org/xmlns "
        "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd";

    if (config.title) {
        graphml.append_child("desc")
            .append_child(pugi::node_cdata)
            .set_value(config.title());
    }

    generate_keys(graphml);

    auto graph_node = make_graph(graphml, "G");

    if (config.using_namespace) {
        add_data(graph_node, "using_namespace",
            config.using_namespace().to_string());
    }

    generate_diagram(graph_node);

    graph.save(ostr, "  ");
}

template <typename C, typename D>
void generator<C, D>::generator::generate_relationships(
    graphml_node_t &parent) const
{
    generator<C, D>::model().for_all_elements([&](auto &&view) {
        for (const auto &el : view)
            generate_relationships(el, parent);
    });
}

template <typename C, typename D>
void generator<C, D>::generate_relationships(
    const model::diagram_element &c, graphml_node_t &parent) const
{
    const auto &model = generator<C, D>::model();

    for (const auto &r : c.relationships()) {
        auto target_element = model.get(r.destination());
        if (!target_element.has_value()) {
            LOG_DBG("Skipping {} relation from '{}' to '{}' due "
                    "to unresolved destination id",
                to_string(r.type()), c.full_name(true),
                r.destination().value());
            continue;
        }

        const auto maybe_target_id =
            node_ids_.get(target_element.value().alias());
        const auto maybe_src_id = node_ids_.get(c.alias());

        if (!maybe_src_id || !maybe_target_id)
            continue;

        auto edge_node = parent.append_child("edge");

        edge_node.append_attribute("id") = fmt::format("e{}", edge_id_++);
        edge_node.append_attribute("source") = *maybe_src_id;
        edge_node.append_attribute("target") = *maybe_target_id;

        add_data(edge_node, "type", to_string(r.type()));
        if (!r.label().empty())
            add_data(edge_node, "label", r.label());

        if (r.access() != access_t::kNone)
            add_data(edge_node, "access", to_string(r.access()));
    }
}

template <typename C, typename D>
template <typename T>
void generator<C, D>::generate_notes(const T &p, graphml_node_t &parent) const
{
    const auto &config = generators::generator<C, D>::config();

    std::vector<std::pair</* element node id */ std::string,
        /* note node id */ std::string>>
        note_id_map;

    // First generate the note XML nodes
    auto note_index{0U};
    for (const auto &e : p) {
        // First try to extract notes from comment decorators (i.e. \uml
        // directives in code comments)
        for (const auto &decorator : e->decorators()) {
            auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
            if (note && note->applies_to_diagram(config.name)) {
                auto note_id = node_ids_.add(
                    fmt::format("{}-N_{}", e->alias(), note_index));
                auto maybe_element_node_id = node_ids_.get(e->alias());
                if (maybe_element_node_id) {
                    auto note_node = make_node(parent, note_id);
                    add_data(note_node, "type", "note");
                    add_data(note_node, "name", note->text, true);
                    note_id_map.emplace_back(*maybe_element_node_id, note_id);
                    note_index++;
                }
            }
        }

        // Now try to extract notes from `graphml` configuration file section
        if (config.graphml &&
            config.graphml().notes.count(e->full_name(false))) {
            for (const auto &note :
                config.graphml().notes.at(e->full_name(false))) {
                auto note_id = node_ids_.add(
                    fmt::format("{}-N_{}", e->alias(), note_index));
                auto maybe_element_node_id = node_ids_.get(e->alias());
                if (maybe_element_node_id) {
                    auto rendered_note = common::jinja::render_template(
                        generator<C, D>::env(), note);
                    if (rendered_note) {
                        auto note_node = make_node(parent, note_id);
                        add_data(note_node, "type", "note");
                        add_data(note_node, "name", *rendered_note, true);
                        note_id_map.emplace_back(
                            *maybe_element_node_id, note_id);
                        note_index++;
                    }
                }
            }
        }
    }

    // Now generate the edges connecting note nodes to element notes
    for (const auto &[element_node_id, note_node_id] : note_id_map) {
        auto edge_node = parent.append_child("edge");

        edge_node.append_attribute("id") = fmt::format("e{}", edge_id_++);
        edge_node.append_attribute("source") = note_node_id;
        edge_node.append_attribute("target") = element_node_id;

        add_data(edge_node, "type", "none");
    }
}

template <typename C, typename D>
void generator<C, D>::generate_metadata(graphml_t &parent) const
{
    if (generators::generator<C, D>::config().generate_metadata()) {
        auto comment = parent.append_child(pugi::node_comment);
        comment.set_value(fmt::format(
            " Generated with clang-uml {} ", clanguml::version::version()));

        comment = parent.append_child(pugi::node_comment);
        comment.set_value(
            fmt::format(" LLVM version {} ", clang::getClangFullVersion()));
    }
}

template <typename C, typename D>
pugi::xml_node generator<C, D>::make_node(
    graphml_node_t &parent, const std::string &id) const
{
    auto result = parent.append_child("node");
    result.append_attribute("id") = id;
    return result;
}

template <typename C, typename D>
pugi::xml_node generator<C, D>::make_graph(
    graphml_node_t &parent, const std::string &id) const
{
    auto result = parent.append_child("graph");
    result.append_attribute("id") = graph_ids_.add(id);
    result.append_attribute("edgedefault").set_value("directed");
    result.append_attribute("parse.nodeids").set_value("canonical");
    result.append_attribute("parse.edgeids").set_value("canonical");
    result.append_attribute("parse.order").set_value("nodesfirst");
    return result;
}

template <typename C, typename D>
pugi::xml_node generator<C, D>::make_subgraph(graphml_node_t &parent,
    const std::string &id, const std::string &name,
    const std::string &type) const
{
    auto graph_node = parent.append_child("node");
    graph_node.append_attribute("id") = node_ids_.add(id);

    if (!name.empty())
        add_data(graph_node, "name", name);

    if (!type.empty())
        add_data(graph_node, "type", type);

    return graph_node;
}

template <typename C, typename D>
void generator<C, D>::add_data(pugi::xml_node &node,
    const std::string &key_name, const std::string &value, bool cdata) const
{
    using namespace std::string_literals;

    std::optional<std::pair<std::string, property_type>> key_id;
    if (node.name() == "node"s)
        key_id = node_properties().get(key_name);
    else if (node.name() == "graph"s)
        key_id = graph_properties().get(key_name);
    else if (node.name() == "edge"s)
        key_id = edge_properties().get(key_name);

    if (key_id.has_value()) {
        auto data = node.append_child("data");
        data.append_attribute("key") = key_id->first;
        if (cdata)
            data.append_child(pugi::node_cdata).set_value(value);
        else
            data.text().set(value);
    }
}

template <typename C, typename D>
void generator<C, D>::add_cdata(pugi::xml_node &node,
    const std::string &key_name, const std::string &value) const
{
    return add_data(node, key_name, value, true);
}

template <typename C, typename D>
template <typename T>
void generator<C, D>::add_url(pugi::xml_node &node, const T &c) const
{
    auto maybe_link_pattern =
        clanguml::common::generators::generator<C, D>::get_link_pattern(c);

    inja::json e_ctx =
        common::jinja::jinja_context<common::model::diagram_element>(c);

    if (maybe_link_pattern) {
        const auto &[link_prefix, link_pattern] = *maybe_link_pattern;
        try {
            auto ec =
                clanguml::common::generators::generator<C, D>::element_context(
                    c, e_ctx);
            common::generators::make_context_source_relative(ec, link_prefix);

            auto url =
                clanguml::common::generators::generator<C, D>::env().render(
                    std::string_view{link_pattern}, ec);

            add_data(node, "url", url);
        }
        catch (const inja::json::parse_error &e) {
            LOG_ERROR("Failed to parse Jinja template: {}", link_pattern);
        }
        catch (const inja::RenderError &e) {
            LOG_DBG("Failed to render GraphML URL: \n{}\n due to: {}",
                link_pattern, e.what());
        }
        catch (const std::exception &e) {
            LOG_ERROR("Failed to render GraphML URL: \n{}\n due to: {}",
                link_pattern, e.what());
        }
    }

    auto maybe_tooltip_pattern =
        clanguml::common::generators::generator<C, D>::get_tooltip_pattern(c);

    if (maybe_tooltip_pattern) {
        const auto &[tooltip_prefix, tooltip_pattern] = *maybe_tooltip_pattern;
        try {
            auto ec =
                clanguml::common::generators::generator<C, D>::element_context(
                    c, e_ctx);
            common::generators::make_context_source_relative(
                ec, tooltip_prefix);

            auto tooltip =
                clanguml::common::generators::generator<C, D>::env().render(
                    std::string_view{tooltip_pattern}, ec);

            add_data(node, "tooltip", tooltip);
        }
        catch (const inja::json::parse_error &e) {
            LOG_ERROR("Failed to parse Jinja template: {}", tooltip_pattern);
        }
        catch (const std::exception &e) {
            LOG_ERROR("Failed to render PlantUML directive: \n{}\n due to: {}",
                tooltip_pattern, e.what());
        }
    }
}

template <typename C, typename D>
void generator<C, D>::generator::generate_key(pugi::xml_node &parent,
    const std::string &attr_name, const std::string &for_value,
    const std::string &id_value, const std::string &attr_type) const
{
    auto key = parent.append_child("key");
    key.append_attribute("attr.name") = attr_name.c_str();
    key.append_attribute("attr.type") = attr_type.c_str();
    key.append_attribute("for") = for_value.c_str();
    key.append_attribute("id") = id_value.c_str();
}

template <typename C, typename D>
void generator<C, D>::generate_keys(graphml_node_t &parent) const
{
    for (const auto &[name, type] : graph_property_names()) {
        const auto [id, t] = graph_properties_.add(name);
        generate_key(parent, name, "graph", id, to_string(type));
    }

    for (const auto &[name, type] : node_property_names()) {
        const auto [id, t] = node_properties_.add(name);
        generate_key(parent, name, "node", id, to_string(type));
    }

    for (const auto &[name, type] : edge_property_names()) {
        const auto [id, t] = edge_properties_.add(name);
        generate_key(parent, name, "edge", id, to_string(type));
    }
}
} // namespace clanguml::common::generators::graphml