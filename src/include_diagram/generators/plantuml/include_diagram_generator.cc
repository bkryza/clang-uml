/**
 * src/package_diagram/generators/plantuml/package_diagram_generator.cc
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

#include "include_diagram_generator.h"

#include "util/error.h"

namespace clanguml::include_diagram::generators::plantuml {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_relationships(
    const source_file &f, std::ostream &ostr) const
{
    LOG_DBG("Generating relationships for file {}", f.full_name(true));
}

void generator::generate(const source_file &f, std::ostream &ostr) const
{
    LOG_DBG("Generating source_file {}", f.name());
}

void generator::generate(std::ostream &ostr) const
{
    ostr << "@startuml" << '\n';

    generate_plantuml_directives(ostr, m_config.puml().before);

    for (const auto &p : m_model) {
        generate(dynamic_cast<source_file &>(*p), ostr);
        ostr << '\n';
    }

    // Process package relationships
    for (const auto &p : m_model) {
        generate_relationships(dynamic_cast<source_file &>(*p), ostr);
        ostr << '\n';
    }

    generate_config_layout_hints(ostr);

    generate_plantuml_directives(ostr, m_config.puml().after);

    ostr << "@enduml" << '\n';
}
}
