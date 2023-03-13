/**
 * src/class_diagram/generators/json/class_diagram_generator.h
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
#pragma once

#include "class_diagram/model/class.h"
#include "class_diagram/model/concept.h"
#include "class_diagram/model/diagram.h"
#include "class_diagram/model/enum.h"
#include "class_diagram/visitor/translation_unit_visitor.h"
#include "common/generators/json/generator.h"
#include "common/generators/nested_element_stack.h"
#include "common/model/relationship.h"
#include "config/config.h"
#include "util/util.h"

#include <glob/glob.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace class_diagram {
namespace generators {
namespace json {

using diagram_config = clanguml::config::class_diagram;
using diagram_model = clanguml::class_diagram::model::diagram;
template <typename C, typename D>
using common_generator = clanguml::common::generators::json::generator<C, D>;

using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::class_element;
using clanguml::class_diagram::model::concept_;
using clanguml::class_diagram::model::enum_;
using clanguml::common::model::access_t;
using clanguml::common::model::package;
using clanguml::common::model::relationship_t;

using namespace clanguml::util;

class generator : public common_generator<diagram_config, diagram_model> {
public:
    generator(diagram_config &config, diagram_model &model);

    void generate(const class_ &c, std::ostream &ostr) const;

    void generate_top_level_elements(std::ostream &ostr) const;

    void generate(std::ostream &ostr) const override;

private:
    std::string render_name(std::string name) const;

    mutable nlohmann::json json_;
};

} // namespace json
} // namespace generators
} // namespace class_diagram
} // namespace clanguml
