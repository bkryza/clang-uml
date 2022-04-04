/**
 * src/include_diagram/generators/plantuml/include_diagram_generator.h
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
#pragma once

#include "common/generators/plantuml/generator.h"
#include "common/model/package.h"
#include "common/model/relationship.h"
#include "config/config.h"
#include "cx/compilation_database.h"
#include "include_diagram/model/diagram.h"
#include "include_diagram/model/source_file.h"
#include "include_diagram/visitor/translation_unit_visitor.h"
#include "util/util.h"

#include <cppast/cpp_entity_index.hpp>
#include <cppast/libclang_parser.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace include_diagram {
namespace generators {
namespace plantuml {

using diagram_config = clanguml::config::include_diagram;
using diagram_model = clanguml::include_diagram::model::diagram;

template <typename C, typename D>
using common_generator =
    clanguml::common::generators::plantuml::generator<C, D>;

using clanguml::common::model::access_t;
using clanguml::common::model::package;
using clanguml::common::model::relationship_t;
using clanguml::include_diagram::model::source_file;
using namespace clanguml::util;

class generator : public common_generator<diagram_config, diagram_model> {
public:
    generator(diagram_config &config, diagram_model &model);

    void generate_relationships(const source_file &p, std::ostream &ostr) const;

    void generate(const source_file &e, std::ostream &ostr) const;

    void generate(std::ostream &ostr) const;
};

}
}
}
}
