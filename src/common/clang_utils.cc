/**
 * src/common/visitor/clang_utils.cc
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

#include "clang_utils.h"

#include <clang/Lex/Preprocessor.h>

namespace clanguml::common {

std::optional<clanguml::common::model::namespace_> get_enclosing_namespace(
    const clang::DeclContext *decl)
{
    if (!decl->getEnclosingNamespaceContext()->isNamespace())
        return {};

    const auto *namespace_declaration =
        clang::cast<clang::NamespaceDecl>(decl->getEnclosingNamespaceContext());

    if (namespace_declaration == nullptr) {
        return {};
    }

    return clanguml::common::model::namespace_{
        common::get_qualified_name(*namespace_declaration)};
}

std::string get_tag_name(const clang::TagDecl &declaration)
{
    auto base_name = declaration.getNameAsString();
    if (base_name.empty()) {
        base_name =
            fmt::format("(anonymous_{})", std::to_string(declaration.getID()));
    }

    if (declaration.getParent() && declaration.getParent()->isRecord()) {
        // If the record is nested within another record (e.g. class or struct)
        // we have to maintain a containment namespace in order to ensure
        // unique names within the diagram
        std::deque<std::string> record_parent_names;
        record_parent_names.push_front(base_name);

        auto *cls_parent{declaration.getParent()};
        while (cls_parent->isRecord()) {
            auto parent_name =
                static_cast<const clang::RecordDecl *>(cls_parent)
                    ->getNameAsString();
            record_parent_names.push_front(parent_name);
            cls_parent = cls_parent->getParent();
        }
        return fmt::format("{}", fmt::join(record_parent_names, "##"));
    }

    return base_name;
}

std::string to_string(const clang::QualType &type, const clang::ASTContext &ctx,
    bool try_canonical)
{
    const clang::PrintingPolicy print_policy(ctx.getLangOpts());

    auto result{type.getAsString(print_policy)};

    if (try_canonical && result.find('<') != std::string::npos) {
        auto canonical_type_name =
            type.getCanonicalType().getAsString(print_policy);

        auto result_qualified_template_name =
            result.substr(0, result.find('<'));
        auto result_template_arguments = result.substr(result.find('<'));

        auto canonical_qualified_template_name =
            canonical_type_name.substr(0, canonical_type_name.find('<'));

        // Choose the longer name (why do I have to do this?)
        if (result_qualified_template_name.size() <
            canonical_qualified_template_name.size()) {

            result =
                canonical_qualified_template_name + result_template_arguments;
        }
    }

    // If for any reason clang reports the type as empty string, make sure
    // it has some default name
    if (result.empty())
        result = "(anonymous)";
    else if (util::contains(result, "unnamed struct")) {
        result = common::get_tag_name(*type->getAsTagDecl());
    }

    // Remove trailing spaces after commas in template arguments
    clanguml::util::replace_all(result, ", ", ",");

    return result;
}

std::string to_string(const clang::RecordType &type,
    const clang::ASTContext &ctx, bool try_canonical)
{
    return to_string(type.desugar(), ctx, try_canonical);
}

std::string get_source_text_raw(
    clang::SourceRange range, const clang::SourceManager &sm)
{
    return clang::Lexer::getSourceText(
        clang::CharSourceRange::getCharRange(range), sm, clang::LangOptions())
        .str();
}

std::string get_source_text(
    clang::SourceRange range, const clang::SourceManager &sm)
{
    clang::LangOptions lo;

    auto start_loc = sm.getSpellingLoc(range.getBegin());
    auto last_token_loc = sm.getSpellingLoc(range.getEnd());
    auto end_loc = clang::Lexer::getLocForEndOfToken(last_token_loc, 0, sm, lo);
    auto printable_range = clang::SourceRange{start_loc, end_loc};
    return get_source_text_raw(printable_range, sm);
}

template <> id_t to_id(const std::string &full_name)
{
    return std::hash<std::string>{}(full_name) >> 3;
}

template <> id_t to_id(const clang::NamespaceDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> id_t to_id(const clang::RecordDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> id_t to_id(const clang::EnumDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> id_t to_id(const clang::TagDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> id_t to_id(const clang::CXXRecordDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> id_t to_id(const clang::EnumType &t) { return to_id(*t.getDecl()); }

template <> id_t to_id(const std::filesystem::path &file)
{
    return to_id(file.lexically_normal().string());
}

template <> id_t to_id(const clang::TemplateArgument &template_argument)
{
    if (template_argument.getKind() == clang::TemplateArgument::Type) {
        if (template_argument.getAsType()->getAs<clang::EnumType>())
            return to_id(*template_argument.getAsType()
                              ->getAs<clang::EnumType>()
                              ->getAsTagDecl());
        else if (template_argument.getAsType()->getAs<clang::RecordType>())
            return to_id(*template_argument.getAsType()
                              ->getAs<clang::RecordType>()
                              ->getAsRecordDecl());
    }

    throw std::runtime_error("Cannot generate id for template argument");
}
}
