/**
 * src/common/visitor/clang_utils.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

model::access_t access_specifier_to_access_t(
    clang::AccessSpecifier access_specifier)
{
    auto access = model::access_t::kPublic;
    switch (access_specifier) {
    case clang::AccessSpecifier::AS_public:
        access = model::access_t::kPublic;
        break;
    case clang::AccessSpecifier::AS_private:
        access = model::access_t::kPrivate;
        break;
    case clang::AccessSpecifier::AS_protected:
        access = model::access_t::kProtected;
        break;
    default:
        break;
    }

    return access;
}

model::namespace_ get_tag_namespace(const clang::TagDecl &declaration)
{
    model::namespace_ ns;

    const auto *parent{declaration.getParent()};

    // First walk up to the nearest namespace, e.g. from nested class or enum
    while ((parent != nullptr) && !parent->isNamespace()) {
        parent = parent->getParent();
    }

    // Now build up the namespace
    std::deque<std::string> namespace_tokens;
    while ((parent != nullptr) && parent->isNamespace()) {
        if (const auto *ns_decl = clang::dyn_cast<clang::NamespaceDecl>(parent);
            ns_decl != nullptr) {
            if (!ns_decl->isInline() && !ns_decl->isAnonymousNamespace())
                namespace_tokens.push_front(ns_decl->getNameAsString());
        }

        parent = parent->getParent();
    }

    for (const auto &ns_token : namespace_tokens) {
        ns |= ns_token;
    }

    return ns;
}

model::namespace_ get_template_namespace(const clang::TemplateDecl &declaration)
{
    model::namespace_ ns{declaration.getQualifiedNameAsString()};
    ns.pop_back();

    return ns;
}

std::string get_tag_name(const clang::TagDecl &declaration)
{
    auto base_name = declaration.getNameAsString();
    if (base_name.empty()) {
        base_name =
            fmt::format("(anonymous_{})", std::to_string(declaration.getID()));
    }

    if ((declaration.getParent() != nullptr) &&
        declaration.getParent()->isRecord()) {
        // If the record is nested within another record (e.g. class or struct)
        // we have to maintain a containment namespace in order to ensure
        // unique names within the diagram
        std::deque<std::string> record_parent_names;
        record_parent_names.push_front(base_name);

        const auto *cls_parent{declaration.getParent()};
        while (cls_parent->isRecord()) {
            if (const auto *record_decl =
                    clang::dyn_cast<clang::RecordDecl>(cls_parent);
                record_decl != nullptr) {
                record_parent_names.push_front(record_decl->getNameAsString());
            }
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
    else if (util::contains(result, "unnamed struct") ||
        util::contains(result, "unnamed union")) {
        result = common::get_tag_name(*type->getAsTagDecl());
    }
    else if (util::contains(result, "anonymous struct") ||
        util::contains(result, "anonymous union")) {
        result = common::get_tag_name(*type->getAsTagDecl());
    }

    // Remove trailing spaces after commas in template arguments
    clanguml::util::replace_all(result, ", ", ",");
    clanguml::util::replace_all(result, "> >", ">>");

    return result;
}

std::string to_string(const clang::RecordType &type,
    const clang::ASTContext &ctx, bool try_canonical)
{
    return to_string(type.desugar(), ctx, try_canonical);
}

std::string to_string(const clang::Expr *expr)
{
    const clang::LangOptions lang_options;
    std::string result;
    llvm::raw_string_ostream ostream(result);
    expr->printPretty(ostream, nullptr, clang::PrintingPolicy(lang_options));

    return result;
}

std::string to_string(const clang::Stmt *stmt)
{
    const clang::LangOptions lang_options;
    std::string result;
    llvm::raw_string_ostream ostream(result);
    stmt->printPretty(ostream, nullptr, clang::PrintingPolicy(lang_options));

    return result;
}

std::string to_string(const clang::FunctionTemplateDecl *decl)
{
    std::vector<std::string> template_parameters;
    // Handle template function
    for (const auto *parameter : *decl->getTemplateParameters()) {
        if (clang::dyn_cast_or_null<clang::TemplateTypeParmDecl>(parameter) !=
            nullptr) {
            const auto *template_type_parameter =
                clang::dyn_cast_or_null<clang::TemplateTypeParmDecl>(parameter);

            std::string template_parameter{
                template_type_parameter->getNameAsString()};

            if (template_type_parameter->isParameterPack())
                template_parameter += "...";

            template_parameters.emplace_back(std::move(template_parameter));
        }
        else {
            // TODO
        }
    }
    return fmt::format("{}<{}>({})", decl->getQualifiedNameAsString(),
        fmt::join(template_parameters, ","), "");
}

std::string to_string(const clang::TypeConstraint *tc)
{
    if (tc == nullptr)
        return {};

    const clang::PrintingPolicy print_policy(
        tc->getNamedConcept()->getASTContext().getLangOpts());

    std::string ostream_buf;
    llvm::raw_string_ostream ostream{ostream_buf};
    tc->print(ostream, print_policy);

    return ostream.str();
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
    const clang::LangOptions lo;

    auto start_loc = sm.getSpellingLoc(range.getBegin());
    auto last_token_loc = sm.getSpellingLoc(range.getEnd());
    auto end_loc = clang::Lexer::getLocForEndOfToken(last_token_loc, 0, sm, lo);
    auto printable_range = clang::SourceRange{start_loc, end_loc};
    return get_source_text_raw(printable_range, sm);
}

bool is_subexpr_of(const clang::Stmt *parent_stmt, const clang::Stmt *sub_stmt)
{
    if (parent_stmt == nullptr || sub_stmt == nullptr)
        return false;

    if (parent_stmt == sub_stmt)
        return true;

    return std::any_of(parent_stmt->child_begin(), parent_stmt->child_end(),
        [sub_stmt](const auto *e) { return is_subexpr_of(e, sub_stmt); });
}

template <> id_t to_id(const std::string &full_name)
{
    return static_cast<id_t>(std::hash<std::string>{}(full_name) >> 3U);
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
        if (const auto *enum_type =
                template_argument.getAsType()->getAs<clang::EnumType>();
            enum_type != nullptr)
            return to_id(*enum_type->getAsTagDecl());

        if (const auto *record_type =
                template_argument.getAsType()->getAs<clang::RecordType>();
            record_type != nullptr)
            return to_id(*record_type->getAsRecordDecl());
    }

    throw std::runtime_error("Cannot generate id for template argument");
}

std::pair<common::model::namespace_, std::string> split_ns(
    const std::string &full_name)
{
    assert(!full_name.empty());

    auto name_before_template = ::clanguml::util::split(full_name, "<")[0];
    auto ns = common::model::namespace_{
        ::clanguml::util::split(name_before_template, "::")};
    auto name = ns.name();
    ns.pop_back();
    return {ns, name};
}

std::vector<common::model::template_parameter> parse_unexposed_template_params(
    const std::string &params,
    const std::function<std::string(const std::string &)> &ns_resolve,
    int depth)
{
    using common::model::template_parameter;

    std::vector<template_parameter> res;

    auto it = params.begin();
    while (std::isspace(*it) != 0)
        ++it;

    std::string type{};
    std::vector<template_parameter> nested_params;
    bool complete_class_template_argument{false};

    while (it != params.end()) {
        if (*it == '<') {
            int nested_level{0};
            auto bracket_match_begin = it + 1;
            auto bracket_match_end = bracket_match_begin;
            while (bracket_match_end != params.end()) {
                if (*bracket_match_end == '<') {
                    nested_level++;
                }
                else if (*bracket_match_end == '>') {
                    if (nested_level > 0)
                        nested_level--;
                    else
                        break;
                }
                else {
                }
                bracket_match_end++;
            }

            std::string nested_params_str(
                bracket_match_begin, bracket_match_end);

            nested_params = parse_unexposed_template_params(
                nested_params_str, ns_resolve, depth + 1);

            if (nested_params.empty()) {
                // We couldn't extract any nested template parameters from
                // `nested_params_str` so just add it as type of template
                // argument as is
                nested_params.emplace_back(
                    template_parameter::make_unexposed_argument(
                        nested_params_str));
            }

            it = bracket_match_end - 1;
        }
        else if (*it == '>') {
            complete_class_template_argument = true;
            if (depth == 0) {
                break;
            }
        }
        else if (*it == ',') {
            complete_class_template_argument = true;
        }
        else {
            type += *it;
        }
        if (complete_class_template_argument) {
            auto t = template_parameter::make_unexposed_argument(
                ns_resolve(clanguml::util::trim(type)));
            type = "";
            for (auto &&param : nested_params)
                t.add_template_param(std::move(param));

            res.emplace_back(std::move(t));
            complete_class_template_argument = false;
        }
        it++;
    }

    if (!type.empty()) {
        auto t = template_parameter::make_unexposed_argument(
            ns_resolve(clanguml::util::trim(type)));
        type = "";
        for (auto &&param : nested_params)
            t.add_template_param(std::move(param));

        res.emplace_back(std::move(t));
    }

    return res;
}
} // namespace clanguml::common
