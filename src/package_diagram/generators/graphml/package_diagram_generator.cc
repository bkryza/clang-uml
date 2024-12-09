/**
 * @file src/package_diagram/generators/graphml/package_diagram_generator.cc
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

#include "package_diagram_generator.h"

#include "util/error.h"

namespace clanguml::package_diagram::generators::graphml {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_top_level_elements(graphml_node_t &parent) const
{
    for (const auto &p : model()) {
        auto &pkg = dynamic_cast<package &>(*p);
        generate(pkg, parent);
    }
}

void generator::generate(const package &p, graphml_node_t &parent) const
{
    LOG_DBG("Generating package {}", p.full_name(false));

    const auto &uns = config().using_namespace();

    if (!uns.starts_with({p.full_name(false)})) {
        auto package_node = make_subgraph(
            parent, p.alias(), p.name(), to_string(config().package_type()));

        add_url(package_node, p);

        if (p.is_deprecated())
            add_data(package_node, "stereotype", "deprecated");

        auto graph_node = make_graph(package_node, p.alias());

        for (const auto &subpackage : p) {
            auto &pkg = dynamic_cast<package &>(*subpackage);
            generate(pkg, graph_node);
        }
    }
    else {
        for (const auto &subpackage : p) {
            auto &pkg = dynamic_cast<package &>(*subpackage);
            generate(pkg, parent);
        }
    }
}

} // namespace clanguml::package_diagram::generators::graphml
