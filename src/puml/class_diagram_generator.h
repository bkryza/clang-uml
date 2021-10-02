/**
 * src/puml/class_diagram_generator.h
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

#include "config/config.h"
#include "cx/compilation_database.h"
#include "uml/class_diagram/model/class.h"
#include "uml/class_diagram/model/class_relationship.h"
#include "uml/class_diagram/model/diagram.h"
#include "uml/class_diagram/model/enum.h"
#include "uml/class_diagram_visitor.h"
#include "util/util.h"

#include <cppast/cpp_entity_index.hpp>
#include <cppast/libclang_parser.hpp>
#include <glob/glob.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace generators {
namespace class_diagram {
namespace puml {

using diagram_config = clanguml::class_diagram::model::diagram;
using diagram_model = clanguml::class_diagram::model::diagram;
using clanguml::class_diagram::model::class_;
using clanguml::class_diagram::model::enum_;
using clanguml::class_diagram::model::relationship_t;
using clanguml::class_diagram::model::scope_t;
using namespace clanguml::util;

std::string relative_to(std::string n, std::string c);

class generator {
public:
    generator(clanguml::config::class_diagram &config, diagram_model &model);

    std::string to_string(scope_t scope) const;

    std::string to_string(relationship_t r, std::string style = "") const;

    std::string name(relationship_t r) const;

    void generate_alias(const class_ &c, std::ostream &ostr) const;

    void generate_alias(const enum_ &e, std::ostream &ostr) const;

    void generate(const class_ &c, std::ostream &ostr) const;

    void generate(const enum_ &e, std::ostream &ostr) const;

    void generate(std::ostream &ostr) const;

    friend std::ostream &operator<<(std::ostream &os, const generator &g);

private:
    clanguml::config::class_diagram &m_config;
    diagram_model &m_model;
};

}

clanguml::class_diagram::model::diagram generate(
    cppast::libclang_compilation_database &db, const std::string &name,
    clanguml::config::class_diagram &diagram);

}
}
}
