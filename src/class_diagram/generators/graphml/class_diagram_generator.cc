/**
 * @file rc/class_diagram/generators/json/class_diagram_generator.cc
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

#include "class_diagram_generator.h"

#include "util/error.h"

namespace clanguml::class_diagram::generators::graphml {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
    init_property_keys();
}

void generator::generate_diagram(graphml_node_t &parent) const
{
    /*
       if (config().using_namespace)
           parent["using_namespace"] = config().using_namespace().to_string();

       if (config().using_module)
           parent["using_module"] = config().using_module();

       if (config().generate_packages.has_value)
           parent["package_type"] = to_string(config().package_type());
       parent["elements"] = std::vector<nlohmann::json>{};
       parent["relationships"] = std::vector<nlohmann::json>{};
   */

    generate_top_level_elements(parent);

    generate_relationships(parent);
}

void generator::generate_key(pugi::xml_node &parent,
    const std::string &attr_name, const std::string &for_value,
    const std::string &id_value, const std::string &attr_type) const
{
    auto key = parent.append_child("key");
    key.append_attribute("attr.name") = attr_name.c_str();
    key.append_attribute("attr.type") = attr_type.c_str();
    key.append_attribute("for") = for_value.c_str();
    key.append_attribute("id") = id_value.c_str();
}

void generator::generate_keys(graphml_node_t &parent) const
{
    generate_key(parent, "id", "graph", *graph_properties().get("id"));
    generate_key(parent, "name", "graph", *graph_properties().get("name"));
    generate_key(parent, "type", "graph", *graph_properties().get("type"));
    generate_key(parent, "using_namespace", "graph",
        *graph_properties().get("using_namespace"));

    // Node properties
    generate_key(parent, "id", "node", *node_properties().get("id"));
    generate_key(parent, "type", "node", *node_properties().get("type"));
    generate_key(parent, "name", "node", *node_properties().get("name"));
    generate_key(
        parent, "stereotype", "node", *node_properties().get("stereotype"));
    generate_key(parent, "url", "node", *node_properties().get("url"));
    generate_key(parent, "tooltip", "node", *node_properties().get("tooltip"));

    // Edge properties
    generate_key(parent, "type", "edge", *edge_properties().get("type"));
    generate_key(parent, "access", "edge", *edge_properties().get("access"));
    generate_key(parent, "label", "edge", *edge_properties().get("label"));
    generate_key(parent, "url", "edge", *edge_properties().get("url"));
}

void generator::generate_top_level_elements(graphml_node_t &parent) const
{

    for (const auto &p : model()) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            if (!pkg->is_empty())
                generate(*pkg, parent);
        }
        else {
            model().dynamic_apply(
                p.get(), [&](auto *el) { generate(*el, parent); });
        }
    }
}

void generator::generate(const package &p, graphml_node_t &parent) const
{
    const auto &uns = config().using_namespace();
    using namespace common::generators::graphml;

    //    nlohmann::json package_object;
    pugi::xml_node package_node;

    if (config().generate_packages()) {
        // Don't generate packages from namespaces filtered out by
        // using_namespace
        if (!uns.starts_with({p.full_name(false)})) {
            LOG_DBG("Generating package {}", p.name());

            package_node = make_subgraph(parent, p.alias(), p.name(),
                to_string(config().package_type()));
        }
    }

    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty()) {
                if (config().generate_packages()) {
                    generate(sp, package_node);
                }
                else
                    generate(sp, parent);
            }
        }
        else {
            model().dynamic_apply(subpackage.get(), [&](auto *el) {
                if (config().generate_packages()) {
                    generate(*el, package_node);
                }
                else
                    generate(*el, parent);
            });
        }
    }

    //    if (config().generate_packages() && !package_object.empty()) {
    //        parent["elements"].push_back(std::move(package_object));
    //    }
}

void generator::generate(const class_ &c, graphml_node_t &parent) const
{
    using namespace common::generators::graphml;

    std::string full_name;
    if (!config().generate_fully_qualified_name())
        full_name = c.full_name_no_ns();
    else
        full_name = c.full_name(true);

    auto class_node = make_node(parent, node_ids_.add(c.alias()));
    add_data(class_node, node_properties().get("type"), c.type_name());
    add_cdata(class_node, node_properties().get("name"),
        config().simplify_template_type(render_name(full_name)));
    if (c.is_abstract()) {
        add_data(class_node, node_properties().get("stereotype"), "abstract");
    }
    if (c.is_union()) {
        add_data(class_node, node_properties().get("stereotype"), "union");
    }
    add_url(class_node, c);
}

void generator::generate(const enum_ &e, graphml_node_t &parent) const
{
    using namespace common::generators::graphml;

    std::string full_name;
    if (!config().generate_fully_qualified_name())
        full_name = e.name();
    else
        full_name = e.full_name(true);

    auto node = make_node(parent, node_ids_.add(e.alias()));
    add_data(node, node_properties().get("type"), e.type_name());
    add_cdata(node, node_properties().get("name"), render_name(full_name));
    add_url(node, e);
}

void generator::generate(const concept_ &c, graphml_node_t &parent) const
{
    using namespace common::generators::graphml;

    std::string full_name;
    if (!config().generate_fully_qualified_name())
        full_name = c.full_name_no_ns();
    else
        full_name = c.full_name(true);

    auto node = make_node(parent, node_ids_.add(c.alias()));
    add_data(node, node_properties().get("type"), c.type_name());
    add_cdata(node, node_properties().get("name"), render_name(full_name));
    add_url(node, c);

    //    nlohmann::json object = c;
    //
    //    if (!config().generate_fully_qualified_name())
    //        object["display_name"] =
    //            common::generators::graphml::render_name(c.full_name_no_ns());
    //
    //    parent["elements"].push_back(std::move(object));
}

void generator::generate(const objc_interface &c, graphml_node_t &parent) const
{
    using namespace common::generators::graphml;

    auto node = make_node(parent, node_ids_.add(c.alias()));
    add_data(node, node_properties().get("type"), c.type_name());
    add_data(
        node, node_properties().get("name"), render_name(c.full_name(true)));
    add_url(node, c);
    //    nlohmann::json object = c;
    //
    //    // Perform config dependent postprocessing on generated class
    //    if (!config().generate_fully_qualified_name())
    //        object["display_name"] =
    //            common::generators::graphml::render_name(c.full_name_no_ns());
    //
    //    object["display_name"] =
    //        config().simplify_template_type(object["display_name"]);
    //
    //    parent["elements"].push_back(std::move(object));
}

void generator::generate_relationships(graphml_node_t &parent) const
{
    for (const auto &p : model()) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            generate_relationships(*pkg, parent);
        }
        else {
            model().dynamic_apply(p.get(),
                [&](auto *el) { generate_relationships(*el, parent); });
        }
    }
}

template <>
void generator::generate_relationships<package>(
    const package &p, graphml_node_t &parent) const
{
    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty())
                generate_relationships(sp, parent);
        }
        else {
            model().dynamic_apply(subpackage.get(), [&](auto *el) {
                if (model().should_include(*el)) {
                    generate_relationships(*el, parent);
                }
            });
        }
    }
}

void generator::init_property_keys()
{
    graph_properties().add("id");
    graph_properties().add("name");
    graph_properties().add("type");
    graph_properties().add("using_namespace");

    node_properties().add("id");
    node_properties().add("type");
    node_properties().add("name");
    node_properties().add("stereotype");
    node_properties().add("url");
    node_properties().add("tooltip");

    edge_properties().add("type");
    edge_properties().add("url");
    edge_properties().add("label");
    edge_properties().add("access");
}

} // namespace clanguml::class_diagram::generators::graphml
