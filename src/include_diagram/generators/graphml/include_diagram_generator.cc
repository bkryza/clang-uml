/**
 * @file src/include_diagram/generators/graphml/include_diagram_generator.cc
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

#include "include_diagram_generator.h"

#include "util/error.h"

namespace clanguml::include_diagram::generators::graphml {

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
        "is_system", common::generators::graphml::property_type::kBoolean);
    return defaults;
}

void generator::generate(const source_file &f, graphml_node_t &parent) const
{
    auto display_name = f.full_name(false);
#if defined(_MSC_VER)
    util::replace_all(display_name, "\\", "/");
#endif

    if (f.type() == common::model::source_file_t::kDirectory) {
        LOG_DBG("Generating directory {}", f.name());

        auto package_node =
            make_subgraph(parent, f.alias(), f.name(), "folder");

        generate_link(package_node, f);

        auto graph_node = make_graph(package_node, f.alias());

        util::for_each(f, [&, this](const auto &file) {
            generate(dynamic_cast<const source_file &>(*file), graph_node);
        });
    }
    else {
        LOG_DBG("Generating file {}", f.name());

        auto file_node = make_node(parent, node_ids_.add(f.alias()));
        add_data(file_node, "type", "file");
        add_cdata(file_node, "name", f.name());

        if (f.type() == common::model::source_file_t::kHeader) {
            add_data(file_node, "stereotype", "header");
            if (f.is_system_header())
                add_data(file_node, "is_system", "true");
        }
        else {
            add_data(file_node, "stereotype", "source");
        }

        generate_link(file_node, f);
    }
}

void generator::generate_top_level_elements(graphml_node_t &parent) const
{
    // Generate files and folders
    util::for_each(model(), [this, &parent](const auto &f) {
        generate(dynamic_cast<source_file &>(*f), parent);
    });
}
} // namespace clanguml::include_diagram::generators::graphml
