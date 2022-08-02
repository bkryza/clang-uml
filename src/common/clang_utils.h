/**
 * src/common/clang_utils.h
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

#include "cx/util.h"
#include "types.h"

#include <clang/AST/RecursiveASTVisitor.h>

#include <string>
#include <filesystem>

namespace clang {
class NamespaceDecl;
}

namespace clanguml::common {

template <typename T> std::string get_qualified_name(const T &declaration)
{
    auto qualified_name = declaration.getQualifiedNameAsString();
    util::replace_all(qualified_name, "(anonymous namespace)", "");
    util::replace_all(qualified_name, "::::", "::");
    return qualified_name;
}

template <typename T> id_t to_id(const T &declaration)
{
    return declaration.getID();
}

template <> id_t to_id(const clang::NamespaceDecl &declaration);

template <> id_t to_id(const clang::EnumType &type);

template <> id_t to_id(const clang::TemplateSpecializationType &type);

template <> id_t to_id(const std::filesystem::path &type);
}
