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
/**
 * @brief Convert @link clang::AccessSpecifier to @link model::access_t
 *
 * @param access_specifier Clang member access specifier
 * @return Enum value of @link model::access_t
 */
model::access_t access_specifier_to_access_t(
    clang::AccessSpecifier access_specifier);

/**
 * @brief Generate full qualified name for @link clang::TagDecl instance
 *
 * @param declaration Input declaration
 * @return String representation including any templates, parameters and
 * attribtues
 */
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

/**
 * @brief Check if an expression is contained in another expression
 *
 * This method returns true if `sub_stmt` is equal to or is contained in the
 * AST subtree of `parent_stmt`
 *
 * @param parent_stmt Parent statement
 * @param sub_stmt Sub statement
 * @return
 */
bool is_subexpr_of(const clang::Stmt *parent_stmt, const clang::Stmt *sub_stmt);

/**
 * @brief Forward template for convertions to ID from various entities
 *
 * These methods provide the main mechanism for generating globally unique
 * identifiers for all elements in the diagrams. The identifiers must be unique
 * between different translation units in order for element relationships to
 * be properly rendered in diagrams.
 *
 * @tparam T Type of entity for which ID should be computed
 * @param declaration Element (e.g. declaration) for which the ID is needed
 * @return Unique ID
 */
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
