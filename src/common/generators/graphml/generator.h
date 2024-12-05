/**
 * @file src/common/generators/json/generator.h
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
#include "common/model/filters/diagram_filter.h"
#include "config/config.h"
#include "util/error.h"
#include "util/util.h"
#include "version/version.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphml.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <clang/Basic/Version.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <glob/glob.hpp>

#include <boost/property_map/dynamic_property_map.hpp>
#include <ostream>

namespace clanguml::common::model {

} // namespace clanguml::common::model

namespace clanguml::common::generators::graphml {

using clanguml::common::model::access_t;
using clanguml::common::model::element;
using clanguml::common::model::message_t;
using clanguml::common::model::relationship_t;

struct vertex_t {
    std::string id;
    std::string type;
    std::string name;
    std::string url;
};

// template <typename T>
using graph_adjacency_list_t =
    boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, vertex_t>;

// template <typename T>
using graph_t =
    boost::labeled_graph<graph_adjacency_list_t, uint64_t, boost::hash_mapS>;

std::string render_name(std::string name);

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
     * @brief Generate diagram model
     *
     * This method must be implemented in subclasses for specific diagram
     * types.
     *
     * @param ostr Output stream
     */
    virtual void generate_diagram(graph_t &parent) const = 0;

    /**
     * @brief Generate metadata element with diagram metadata
     *
     * @param parent Root JSON object
     */
    void generate_metadata(graph_t &parent) const;
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
    //    const auto &config = generators::generator<C, D>::config();
    //    const auto &model = generators::generator<C, D>::model();
    //
    //    if (!config.allow_empty_diagrams() && model.is_empty()) {
    //        throw clanguml::error::empty_diagram_error{model.type(),
    //        model.name(),
    //            "Diagram configuration resulted in empty diagram."};
    //    }
    //
    //    nlohmann::json j;
    //    j["name"] = model.name();
    //    j["diagram_type"] = to_string(model.type());
    //    if (config.title) {
    //        j["title"] = config.title();
    //    }
    //
    graph_t graph;

    generate_diagram(graph);
    //
    //    generate_metadata(j);
    //
    //    ostr << j;
    boost::dynamic_properties dp;
    dp.property("id", get(&vertex_t::id, graph));
    dp.property("type", get(&vertex_t::type, graph));
    dp.property("name", get(&vertex_t::name, graph));
    dp.property("url", get(&vertex_t::url, graph));

    std::stringstream ss;
    boost::write_graphml(ss, graph, dp, false);

    LOG_ERROR("LABLED_GRAPH: {}", ss.str());

    boost::write_graphml(ostr, graph, dp, false);
}

template <typename C, typename D>
void generator<C, D>::generate_metadata(graph_t &parent) const
{
    //    if (generators::generator<C, D>::config().generate_metadata()) {
    //        parent["metadata"]["clang_uml_version"] =
    //        clanguml::version::version(); parent["metadata"]["schema_version"]
    //        =
    //            clanguml::version::json_generator_schema_version();
    //        parent["metadata"]["llvm_version"] = clang::getClangFullVersion();
    //    }
}

} // namespace clanguml::common::generators::graphml