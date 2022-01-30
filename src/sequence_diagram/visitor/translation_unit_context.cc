/**
 * src/sequence_diagram/visitor/translation_unit_context.cc
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

#include "translation_unit_context.h"

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

namespace clanguml::sequence_diagram::visitor {

translation_unit_context::translation_unit_context(
    cppast::cpp_entity_index &idx,
    clanguml::sequence_diagram::model::diagram &diagram,
    const clanguml::config::sequence_diagram &config)
    : entity_index_{idx}
    , diagram_{diagram}
    , config_{config}
{
}

void translation_unit_context::push_namespace(const std::string &ns)
{
    namespace_.push_back(ns);
}

void translation_unit_context::pop_namespace() { namespace_.pop_back(); }

const std::vector<std::string> &translation_unit_context::get_namespace() const
{
    return namespace_;
}

const clanguml::config::sequence_diagram &
translation_unit_context::config() const
{
    return config_;
}

clanguml::sequence_diagram::model::diagram &translation_unit_context::diagram()
{
    return diagram_;
}

const cppast::cpp_entity_index &translation_unit_context::entity_index() const
{
    return entity_index_;
}

void translation_unit_context::set_current_method(cx::cursor method)
{
    current_method_ = method;
}

cx::cursor &translation_unit_context::current_method()
{
    return current_method_;
}

}
