/**
 * src/uml/sequence_diagram_visitor.h
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
#include "cx/cursor.h"
#include "sequence_diagram_model.h"

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>

namespace clanguml {
namespace visitor {
namespace sequence_diagram {

struct tu_context {
    tu_context(clanguml::model::sequence_diagram::diagram &d_,
        const clanguml::config::sequence_diagram &config_);

    std::vector<std::string> namespace_;
    cx::cursor current_method;
    clanguml::model::sequence_diagram::diagram &d;
    const clanguml::config::sequence_diagram &config;
};

enum CXChildVisitResult translation_unit_visitor(
    CXCursor cx_cursor, CXCursor cx_parent, CXClientData client_data);

}
}
}
