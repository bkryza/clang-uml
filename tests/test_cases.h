/**
 * tests/test_cases.h
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
#include "puml/class_diagram_generator.h"
#include "puml/sequence_diagram_generator.h"
#include "uml/class_diagram_model.h"
#include "uml/class_diagram_visitor.h"
#include "uml/sequence_diagram_visitor.h"

#include "catch.h"

#include <complex>
#include <filesystem>
#include <string>

using Catch::Matchers::Contains;
using Catch::Matchers::EndsWith;
using Catch::Matchers::StartsWith;
using Catch::Matchers::VectorContains;
using clanguml::cx::compilation_database;

std::pair<clanguml::config::config, compilation_database> load_config(
    const std::string &test_name);

clanguml::model::sequence_diagram::diagram generate_sequence_diagram(
    compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram);

clanguml::model::class_diagram::diagram generate_class_diagram(
    compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram);

std::string generate_sequence_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::model::sequence_diagram::diagram &model);

std::string generate_class_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::model::class_diagram::diagram &model);

void save_puml(const std::string &path, const std::string &puml);
