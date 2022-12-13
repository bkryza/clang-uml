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

#include <deque>
#include <filesystem>
#include <string>

namespace clang {
class NamespaceDecl;
}

namespace clanguml::common {
std::string get_tag_name(const clang::TagDecl &declaration);

template <typename T> std::string get_qualified_name(const T &declaration)
{
    auto qualified_name = declaration.getQualifiedNameAsString();
    util::replace_all(qualified_name, "(anonymous namespace)", "");
    util::replace_all(qualified_name, "::::", "::");

    if constexpr (std::is_base_of_v<clang::TagDecl, T>) {
        auto base_name = get_tag_name(declaration);
        model::namespace_ ns{qualified_name};
        ns.pop_back();
        ns = ns | base_name;

        return ns.to_string();
    }

    return qualified_name;
}

model::namespace_ get_tag_namespace(const clang::TagDecl &declaration);

std::optional<clanguml::common::model::namespace_> get_enclosing_namespace(
    const clang::DeclContext *decl);

std::string to_string(const clang::QualType &type, const clang::ASTContext &ctx,
    bool try_canonical = true);

std::string to_string(const clang::RecordType &type,
    const clang::ASTContext &ctx, bool try_canonical = true);

std::string to_string(const clang::Expr *expr);

std::string to_string(const clang::Stmt *stmt);

std::string to_string(const clang::FunctionTemplateDecl *decl);

std::string get_source_text_raw(
    clang::SourceRange range, const clang::SourceManager &sm);

std::string get_source_text(
    clang::SourceRange range, const clang::SourceManager &sm);

bool is_subexpr_of(const clang::Stmt *parent_stmt, const clang::Stmt *sub_stmt);

template <typename T> id_t to_id(const T &declaration);

template <> id_t to_id(const std::string &full_name);

template <> id_t to_id(const clang::NamespaceDecl &declaration);

template <> id_t to_id(const clang::CXXRecordDecl &declaration);

template <> id_t to_id(const clang::EnumDecl &declaration);

template <> id_t to_id(const clang::TagDecl &declaration);

template <> id_t to_id(const clang::EnumType &type);

template <> id_t to_id(const clang::TemplateSpecializationType &type);

template <> id_t to_id(const std::filesystem::path &type);
}
