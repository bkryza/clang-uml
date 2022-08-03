/**
 * src/cx/util.h
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

#include "common/model/namespace.h"

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include <class_diagram/model/template_parameter.h>
#include <string>

namespace clanguml::cx::util {

std::pair<common::model::namespace_, std::string> split_ns(
    const std::string &full_name);

std::vector<class_diagram::model::template_parameter>
parse_unexposed_template_params(const std::string &params,
    std::function<std::string(const std::string &)> ns_resolve);

} // namespace clanguml::cx::util
