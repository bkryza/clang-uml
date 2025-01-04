/**
 * @file rc/class_diagram/generators/json/class_diagram_generator.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

using clanguml::common::to_string;
using clanguml::common::generators::display_name_adapter;

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

std::vector<std::pair<std::string, common::generators::graphml::property_type>>
generator::node_property_names() const
{
    auto defaults =
        common_generator<diagram_config, diagram_model>::node_property_names();
    defaults.emplace_back(
        "is_template", common::generators::graphml::property_type::kBoolean);
    return defaults;
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

    pugi::xml_node package_node;
    pugi::xml_node graph_node;

    if (config().generate_packages()) {
        // Don't generate packages from namespaces filtered out by
        // using_namespace
        if (!uns.starts_with({p.full_name(false)})) {
            auto name = display_name_adapter(p).with_packages().name();
            LOG_DBG("Generating package {}", name);

            package_node = make_subgraph(
                parent, p.alias(), name, to_string(config().package_type()));
            graph_node = make_graph(package_node, p.alias());
        }
    }

    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty()) {
                if (config().generate_packages()) {
                    generate(sp, graph_node);
                }
                else
                    generate(sp, parent);
            }
        }
        else {
            model().dynamic_apply(subpackage.get(), [&](auto *el) {
                if (config().generate_packages()) {
                    generate(*el, graph_node);
                }
                else
                    generate(*el, parent);
            });
        }
    }

    // When generating packages as subgraphs, we want to render the note nodes
    // and their relationship to their elements within the subgraph
    if (config().generate_packages()) {
        generate_notes(p, package_node);
    }
}

void generator::generate(const class_ &c, graphml_node_t &parent) const
{
    using namespace common::generators::graphml;

    std::string full_name;
    if (!config().generate_fully_qualified_name())
        full_name = display_name_adapter(c).with_packages().full_name_no_ns();
    else
        full_name = display_name_adapter(c).full_name(true);

    auto class_node = make_node(parent, node_ids_.add(c.alias()));
    add_data(class_node, "type", c.type_name());
    add_cdata(class_node, "name", config().simplify_template_type(full_name));
    if (c.is_abstract()) {
        add_data(class_node, "stereotype", "abstract");
    }
    else if (c.is_union()) {
        add_data(class_node, "stereotype", "union");
    }
    add_data(class_node, "is_template", to_string(c.is_template()));

    generate_link(class_node, c);
}

void generator::generate(const enum_ &e, graphml_node_t &parent) const
{
    using namespace common::generators::graphml;

    std::string full_name;
    if (!config().generate_fully_qualified_name())
        full_name = display_name_adapter(e).with_packages().name();
    else
        full_name = display_name_adapter(e).full_name(true);

    auto node = make_node(parent, node_ids_.add(e.alias()));
    add_data(node, "type", e.type_name());
    add_cdata(node, "name", full_name);
    generate_link(node, e);
}

void generator::generate(const concept_ &c, graphml_node_t &parent) const
{
    using namespace common::generators::graphml;

    std::string full_name;
    if (!config().generate_fully_qualified_name())
        full_name = display_name_adapter(c).with_packages().full_name_no_ns();
    else
        full_name = display_name_adapter(c).full_name(true);

    auto node = make_node(parent, node_ids_.add(c.alias()));
    add_data(node, "type", c.type_name());
    add_cdata(node, "name", full_name);
    generate_link(node, c);
}

void generator::generate(const objc_interface &c, graphml_node_t &parent) const
{
    using namespace common::generators::graphml;

    auto node = make_node(parent, node_ids_.add(c.alias()));
    add_data(node, "type", c.type_name());
    add_data(node, "name", display_name_adapter(c).full_name(true));
    generate_link(node, c);
}

} // namespace clanguml::class_diagram::generators::graphml
