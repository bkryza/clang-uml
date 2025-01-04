/**
 * @file src/include_diagram/generators/json/include_diagram_generator.cc
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

#include "include_diagram_generator.h"

#include "util/error.h"

namespace clanguml::include_diagram::generators::json {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_relationships(
    const source_file &f, nlohmann::json &parent) const
{
    LOG_DBG("Generating relationships for file {}", f.full_name(true));

    if (f.type() == common::model::source_file_t::kDirectory) {
        util::for_each(f, [this, &parent](const auto &file) {
            generate_relationships(
                dynamic_cast<const source_file &>(*file), parent);
        });
    }
    else {
        for (const auto &r : f.relationships()) {
            nlohmann::json rel = r;
            rel["source"] = std::to_string(f.id().value());
            parent["relationships"].push_back(std::move(rel));
        }
    }
}

void generator::generate(const source_file &f, nlohmann::json &parent) const
{
    if (config().generate_packages && !config().generate_packages()) {
        generate_without_packages(f, parent);
    }
    else {
        generate_with_packages(f, parent);
    }
}

void generator::generate_with_packages(
    const source_file &f, nlohmann::json &parent) const
{
    nlohmann::json j;
    j["id"] = std::to_string(f.id().value());
    j["name"] = f.name();
    auto display_name = f.full_name(false);
#if defined(_MSC_VER)
    util::replace_all(display_name, "\\", "/");
#endif
    j["display_name"] = std::move(display_name);

    if (f.type() == common::model::source_file_t::kDirectory) {
        LOG_DBG("Generating directory {}", f.name());

        j["type"] = "folder";

        util::for_each(f, [this, &j](const auto &file) {
            generate(dynamic_cast<const source_file &>(*file), j);
        });

        parent["elements"].push_back(std::move(j));
    }
    else {
        LOG_DBG("Generating file {}", f.name());

        j["type"] = "file";
        j["file_kind"] = to_string(f.type());
        if (f.type() == common::model::source_file_t::kHeader) {
            j["is_system"] = f.is_system_header();
        }

        parent["elements"].push_back(std::move(j));
    }
}

void generator::generate_without_packages(
    const source_file &f, nlohmann::json &parent) const
{
    nlohmann::json j;
    j["id"] = std::to_string(f.id().value());
    j["name"] = f.file_relative();
    auto display_name = f.full_name(false);
#if defined(_MSC_VER)
    util::replace_all(display_name, "\\", "/");
#endif
    j["display_name"] = std::move(display_name);

    if (f.type() == common::model::source_file_t::kDirectory) {
        util::for_each(f, [this, &parent](const auto &file) {
            generate(dynamic_cast<const source_file &>(*file), parent);
        });
    }
    else {
        LOG_DBG("Generating file {}", f.file_relative());

        j["type"] = "file";
        j["file_kind"] = to_string(f.type());
        if (f.type() == common::model::source_file_t::kHeader) {
            j["is_system"] = f.is_system_header();
        }

        parent["elements"].push_back(std::move(j));
    }
}

void generator::generate_diagram(nlohmann::json &parent) const
{
    parent["elements"] = std::vector<nlohmann::json>{};
    parent["relationships"] = std::vector<nlohmann::json>{};

    // Generate files and folders
    util::for_each(model(), [this, &parent](const auto &f) {
        generate(dynamic_cast<source_file &>(*f), parent);
    });

    // Process file include relationships
    util::for_each(model(), [this, &parent](const auto &f) {
        generate_relationships(dynamic_cast<source_file &>(*f), parent);
    });
}
} // namespace clanguml::include_diagram::generators::json
