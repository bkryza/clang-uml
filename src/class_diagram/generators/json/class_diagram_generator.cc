/**
 * src/class_diagram/generators/json/class_diagram_generator.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

namespace clanguml::class_diagram::generators::json {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate(std::ostream &ostr) const
{
    generate_top_level_elements(ostr);

    ostr << json_;
}

void generator::generate_top_level_elements(std::ostream &ostr) const
{
    for (const auto &p : m_model) {
        if (auto *cls = dynamic_cast<class_ *>(p.get()); cls) {
            if (m_model.should_include(*cls)) {
                generate(*cls, ostr);
            }
        }
    }
}

void generator::generate(const class_ &c, std::ostream & /*ostr*/) const
{

    nlohmann::json object;
    object["id"] = std::to_string(c.id());
    object["name"] = c.name();
    object["namespace"] = c.get_namespace().to_string();
    object["type"] = c.type_name();
    object["display_name"] = c.full_name(false);
    json_["elements"].push_back(std::move(object));
}

} // namespace clanguml::class_diagram::generators::plantuml
