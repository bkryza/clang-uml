/**
 * @file src/include_diagram/generators/mermaid/include_diagram_generator.cc
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

namespace clanguml::include_diagram::generators::mermaid {

using clanguml::common::generators::mermaid::indent;

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_diagram_type(std::ostream &ostr) const
{
    ostr << "flowchart\n";
}

void generator::generate_relationships(
    const source_file &f, std::ostream &ostr) const
{
    if (!util::contains(m_generated_aliases, f.alias()))
        return;

    LOG_DBG("Generating relationships for file {}", f.full_name(true));

    if (f.type() == common::model::source_file_t::kDirectory) {
        util::for_each(f, [this, &ostr](const auto &file) {
            generate_relationships(
                dynamic_cast<const source_file &>(*file), ostr);
        });
    }
    else {
        util::for_each_if(
            f.relationships(),
            [this](const auto &r) {
                return model().should_include(r.type()) &&
                    util::contains(m_generated_aliases,
                        model().get(r.destination()).value().alias());
            },
            [&f, &ostr, this](const auto &r) {
                ostr << indent(1) << f.alias() << " "
                     << (r.type() == common::model::relationship_t::kDependency
                                ? "-.->"
                                : "-->")
                     << " " << model().get(r.destination()).value().alias()
                     << '\n';
            });
    }
}

void generator::generate(const source_file &f, std::ostream &ostr) const
{
    if (f.type() == common::model::source_file_t::kDirectory) {
        LOG_DBG("Generating directory {}", f.name());

        ostr << indent(1) << "subgraph " << f.alias() << "[" << f.name()
             << "]\n";

        util::for_each(f, [this, &ostr](const auto &file) {
            generate(dynamic_cast<const source_file &>(*file), ostr);
        });

        ostr << indent(1) << "end" << '\n';

        m_generated_aliases.emplace(f.alias());
    }
    else {
        LOG_DBG("Generating file {}", f.name());

        if (model().should_include(f)) {
            ostr << indent(1) << f.alias() << "[" << f.name() << "]\n";

            m_generated_aliases.emplace(f.alias());
        }
    }

    if (config().generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, f);
    }
}

void generator::generate_diagram(std::ostream &ostr) const
{
    // Generate files and folders
    util::for_each(model(), [this, &ostr](const auto &f) {
        generate(dynamic_cast<source_file &>(*f), ostr);
    });

    // Process file include relationships
    util::for_each(model(), [this, &ostr](const auto &f) {
        generate_relationships(dynamic_cast<source_file &>(*f), ostr);
    });

    // Process file notes
    util::for_each(model(), [this, &ostr](const auto &f) {
        generate_notes(ostr, dynamic_cast<source_file &>(*f));
    });
}
} // namespace clanguml::include_diagram::generators::mermaid
