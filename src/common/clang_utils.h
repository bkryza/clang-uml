/**
 * @file src/common/clang_utils.h
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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

#include "common/model/enums.h"
#include "common/model/namespace.h"
#include "common/model/template_parameter.h"
#include "config/config.h"
#include "types.h"
#include "util/util.h"

#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <deque>
#include <filesystem>
#include <string>

namespace clang {
class NamespaceDecl;
}

namespace clanguml::common {
/**
 * @brief Convert `clang::AccessSpecifier` to @see clanguml::model::access_t
 *
 * @param access_specifier Clang member access specifier
 * @return Enum value of @see clanguml::model::access_t
 */
model::access_t access_specifier_to_access_t(
    clang::AccessSpecifier access_specifier);

/**
 * @brief Generate full qualified name for
 * [clang::TagDecl](https://clang.llvm.org/doxygen/classclang_1_1TagDecl.html)
 * instance
 *
 * @param declaration Input declaration
 * @return String representation including any templates, parameters and
 *         attribtues
 */
std::string get_tag_name(const clang::TagDecl &declaration);

/**
 * @brief Get qualified name of some Clang declaration
 *
 * This template is convenient for getting qualified name of various types of
 * clang declarations.
 *
 * @tparam T Type of Clang's declaration, e.g. `clang::TagDecl`
 * @param declaration Reference to a clang declaration
 * @return Fully qualified name
 */
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

/**
 * Get namespace of a specific `clang::TagDecl`
 *
 * @param declaration Reference to clang::TagDecl
 * @return Namespace instance
 */
model::namespace_ get_tag_namespace(const clang::TagDecl &declaration);

/**
 * Get namespace of a specific `clang::TemplateDecl`
 *
 * @param declaration Reference to clang::TemplateDecl
 * @return Namespace instance
 */
model::namespace_ get_template_namespace(
    const clang::TemplateDecl &declaration);

std::string to_string(const clang::QualType &type, const clang::ASTContext &ctx,
    bool try_canonical = true);

std::string to_string(const clang::RecordType &type,
    const clang::ASTContext &ctx, bool try_canonical = true);

std::string to_string(
    const clang::TemplateArgument &arg, const clang::ASTContext *ctx = nullptr);

std::string to_string(const clang::Expr *expr);

std::string to_string(const clang::ValueDecl *val);

std::string to_string(const clang::Stmt *stmt);

std::string to_string(const clang::FunctionTemplateDecl *decl);

std::string to_string(const clang::TypeConstraint *tc);

std::string to_string(const clang::TemplateName &templ);

/**
 * @brief Get raw text of specific source range
 *
 * @param range Source range
 * @param sm Source manager reference
 * @return Raw source text
 */
std::string get_source_text_raw(
    clang::SourceRange range, const clang::SourceManager &sm);

/**
 * @brief Get printable range of text of specific source range
 *
 * @param range Source range
 * @param sm Source manager reference
 * @return Printable source text
 */
std::string get_source_text(
    clang::SourceRange range, const clang::SourceManager &sm);

/**
 * @brief Extract template depth and index
 *
 * This function extracts template depth and index values from Clang's
 * `type-parameter-` names.
 *
 * @param type_parameter Clang's type parameter string
 * @return (depth, index, qualifier)
 */
std::tuple<unsigned int, unsigned int, std::string>
extract_template_parameter_index(const std::string &type_parameter);

void ensure_lambda_type_is_relative(
    const config::diagram &config, std::string &parameter_type);

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

/** @defgroup to_id Forward template for convertions to ID from various entities
 *
 * These methods provide the main mechanism for generating globally unique
 * identifiers for all elements in the diagrams. The identifiers must be unique
 * between different translation units in order for element relationships to
 * be properly rendered in diagrams.
 *
 * @{
 */
template <typename T> eid_t to_id(const T &declaration);

template <> eid_t to_id(const std::string &full_name);

eid_t to_id(const clang::QualType &type, const clang::ASTContext &ctx);

template <> eid_t to_id(const clang::NamespaceDecl &declaration);

template <> eid_t to_id(const clang::CXXRecordDecl &declaration);

template <> eid_t to_id(const clang::RecordDecl &declaration);

template <> eid_t to_id(const clang::EnumDecl &declaration);

template <> eid_t to_id(const clang::TagDecl &declaration);

template <> eid_t to_id(const clang::EnumType &type);

template <> eid_t to_id(const clang::TemplateSpecializationType &type);

template <> eid_t to_id(const std::filesystem::path &type);
/** @} */ // end of to_id

/**
 * @brief Split qualified name to namespace and name
 *
 * @param full_name Fully qualified element name
 * @return (namespace, name)
 */
std::pair<common::model::namespace_, std::string> split_ns(
    const std::string &full_name);

/**
 * @brief Parse unexposed (available as string) template params
 *
 * @param params String parameters as provided by Clang
 * @param ns_resolve Namespace resolver function
 * @param depth Current depth in the template specification
 * @return Parsed template parameter
 */
std::vector<common::model::template_parameter> parse_unexposed_template_params(
    const std::string &params,
    const std::function<std::string(const std::string &)> &ns_resolve,
    int depth = 0);

std::vector<std::string> tokenize_unexposed_template_parameter(
    const std::string &t);

template <typename T, typename P, typename F>
void if_dyn_cast(P pointer, F &&func)
{
    if (pointer == nullptr)
        return;

    if (const auto *dyn_cast_value = clang::dyn_cast<T>(pointer);
        dyn_cast_value) {
        std::forward<F>(func)(dyn_cast_value);
    }
}

bool parse_source_location(const std::string &location_str, std::string &file,
    unsigned &line, unsigned &column);

bool is_type_parameter(const std::string &t);

bool is_qualifier(const std::string &q);

bool is_bracket(const std::string &b);

bool is_identifier_character(char c);

bool is_identifier(const std::string &t);

bool is_qualified_identifier(const std::string &t);

bool is_type_token(const std::string &t);

std::string format_condition_text(const std::string &condition_text);

std::string get_condition_text(clang::SourceManager &sm, clang::IfStmt *stmt);

std::string get_condition_text(
    clang::SourceManager &sm, clang::WhileStmt *stmt);

std::string get_condition_text(
    clang::SourceManager &sm, clang::CXXForRangeStmt *stmt);

std::string get_condition_text(clang::SourceManager &sm, clang::ForStmt *stmt);

std::string get_condition_text(clang::SourceManager &sm, clang::DoStmt *stmt);

std::string get_condition_text(
    clang::SourceManager &sm, clang::ConditionalOperator *stmt);

clang::QualType dereference(clang::QualType type);

/**
 * @brief Extract type context and return raw type
 *
 * This function removes the context for a type, for example for:
 * `std::string const&`
 * it will return
 * `(std::string, [const&])`
 *
 * @param type Type to process
 * @return (type, [qualifiers])
 */
std::pair<clang::QualType, std::deque<common::model::context>>
consume_type_context(clang::QualType type);

/**
 * @brief Extract a comment before or next to a statement
 *
 * @param sm clang::SourceManager reference
 * @param context clang::ASTContext reference
 * @param stmt Pointer to the current clang::Stmt
 * @return Pointer to a clang::RawComment* or nullptr
 */
clang::RawComment *get_expression_raw_comment(const clang::SourceManager &sm,
    const clang::ASTContext &context, const clang::Stmt *stmt);

/**
 * Check if function or method declaration is a C++20 coroutine.
 *
 * @param decl Function declaration
 * @return True, if the function is a C++20 coroutine.
 */
bool is_coroutine(const clang::FunctionDecl &decl);

/**
 * Check if named declaration is a C++ struct.
 *
 * @param decl Declaration to check
 * @return True, if declaration represents a struct.
 */
bool is_struct(const clang::NamedDecl *decl);

/**
 * Check if function declaration contains specified attributed
 *
 * @param decl Function declaration
 * @param function_attr Clang function attribute
 * @return True, if decl contains specified function attribute
 */
bool has_attr(const clang::FunctionDecl *decl, clang::attr::Kind function_attr);

/**
 * If `type` is a constant array, return it's number of elements. Otherwise
 * nothing.
 *
 * @param type
 * @return Number of elements in the array.
 */
std::optional<size_t> get_array_size(const clang::ArrayType &type);
} // namespace clanguml::common
