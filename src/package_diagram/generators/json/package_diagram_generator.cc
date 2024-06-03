/**
 * @file src/package_diagram/generators/json/package_diagram_generator.cc
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

namespace clanguml::package_diagram::generators::json {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_relationships(
    const package &p, nlohmann::json &parent) const
{
    LOG_DBG("Generating relationships for package {}", p.full_name(true));

    // Generate this packages relationship
    if (model().should_include(relationship_t::kDependency)) {
        for (const auto &r : p.relationships()) {
            nlohmann::json rel = r;

            auto destination_package = model().get(r.destination());

            if (!destination_package ||
                !model().should_include(
                    dynamic_cast<const package &>(*destination_package)))
                continue;

            rel["source"] = std::to_string(p.id().value());
            parent["relationships"].push_back(std::move(rel));
        }
    }

    // Process it's subpackages relationships
    for (const auto &subpackage : p) {
        generate_relationships(
            dynamic_cast<const package &>(*subpackage), parent);
    }
}

void generator::generate(const package &p, nlohmann::json &parent) const
{
    LOG_DBG("Generating package {}", p.full_name(false));

    const auto &uns = config().using_namespace();
    if (!uns.starts_with({p.full_name(false)})) {
        nlohmann::json j;
        j["id"] = std::to_string(p.id().value());
        j["name"] = p.name();
        j["type"] = to_string(config().package_type());
        j["display_name"] = p.name();
        switch (config().package_type()) {
        case config::package_type_t::kNamespace:
            j["namespace"] = p.get_namespace().to_string();
            break;
        case config::package_type_t::kModule:
            j["namespace"] = p.get_namespace().to_string();
            break;
        case config::package_type_t::kDirectory:
            j["path"] = p.get_namespace().to_string();
            break;
        }

        j["is_deprecated"] = p.is_deprecated();
        if (!p.file().empty())
            j["source_location"] =
                dynamic_cast<const common::model::source_location &>(p);
        if (const auto &comment = p.comment(); comment)
            j["comment"] = comment.value();

        for (const auto &subpackage : p) {
            auto &pkg = dynamic_cast<package &>(*subpackage);
            if (model().should_include(pkg)) {
                generate(pkg, j);
            }
        }

        parent["elements"].push_back(std::move(j));
    }
    else {
        for (const auto &subpackage : p) {
            auto &pkg = dynamic_cast<package &>(*subpackage);
            if (model().should_include(pkg)) {
                generate(pkg, parent);
            }
        }
    }
}

void generator::generate_diagram(nlohmann::json &parent) const
{
    if (config().using_namespace)
        parent["using_namespace"] = config().using_namespace().to_string();
    if (config().using_module)
        parent["using_module"] = config().using_module();

    parent["name"] = model().name();
    parent["diagram_type"] = "package";
    parent["package_type"] = to_string(config().package_type());
    parent["elements"] = std::vector<nlohmann::json>{};
    parent["relationships"] = std::vector<nlohmann::json>{};

    for (const auto &p : model()) {
        auto &pkg = dynamic_cast<package &>(*p);
        if (model().should_include(pkg)) {
            generate(pkg, parent);
        }
    }

    // Process package relationships
    for (const auto &p : model()) {
        if (model().should_include(dynamic_cast<package &>(*p)))
            generate_relationships(dynamic_cast<package &>(*p), parent);
    }
}

} // namespace clanguml::package_diagram::generators::json
