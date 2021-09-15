/**
 * src/puml/sequence_diagram_generator.h
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
#include "uml/sequence_diagram_model.h"
#include "uml/sequence_diagram_visitor.h"
#include "util/util.h"

#include <glob/glob.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace clanguml {
namespace generators {
namespace sequence_diagram {
namespace puml {

using diagram_model = clanguml::model::sequence_diagram::diagram;

class generator {
public:
    generator(clanguml::config::sequence_diagram &config, diagram_model &model);

    std::string to_string(clanguml::model::sequence_diagram::message_t r) const;

    void generate_call(const clanguml::model::sequence_diagram::message &m,
        std::ostream &ostr) const;

    void generate_return(const clanguml::model::sequence_diagram::message &m,
        std::ostream &ostr) const;

    void generate_activity(const clanguml::model::sequence_diagram::activity &a,
        std::ostream &ostr) const;

    void generate(std::ostream &ostr) const;

    friend std::ostream &operator<<(std::ostream &os, const generator &g);

private:
    clanguml::config::sequence_diagram &m_config;
    clanguml::model::sequence_diagram::diagram &m_model;
};

}

clanguml::model::sequence_diagram::diagram generate(
    clanguml::cx::compilation_database &db, const std::string &name,
    clanguml::config::sequence_diagram &diagram);

}
}
}
