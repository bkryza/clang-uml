/**
 * @file src/common/clang_utils.cc
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

std::string to_string(const clang::ArrayType &array_type,
    const clang::ASTContext &ctx, bool try_canonical,
    std::vector<std::string> &dimensions)
{
    auto maybe_size = get_array_size(array_type);
    std::string array_size =
        maybe_size.has_value() ? std::to_string(maybe_size.value()) : "";
    dimensions.emplace_back(std::move(array_size));

    const auto underlying_type = array_type.getElementType();

    if (underlying_type->isArrayType())
        return to_string(*underlying_type->getAsArrayTypeUnsafe(), ctx,
            try_canonical, dimensions);

    std::string dimensions_str;
    for (const auto &d : dimensions) {
        dimensions_str += fmt::format("[{}]", d);
    }
    return fmt::format(
        "{}{}", to_string(underlying_type, ctx, try_canonical), dimensions_str);
}

std::string to_string(const clang::QualType &type, const clang::ASTContext &ctx,
    bool try_canonical)
{
    if (type->isArrayType()) {
        std::vector<std::string> dimensions;
        return to_string(
            *type->getAsArrayTypeUnsafe(), ctx, try_canonical, dimensions);
    }

    clang::PrintingPolicy print_policy(ctx.getLangOpts());
    print_policy.SuppressScope = 0;
    print_policy.PrintCanonicalTypes = 0;

    std::string result;

    result = type.getAsString(print_policy);

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
        const auto *declarationTag = type->getAsTagDecl();
        if (declarationTag == nullptr) {
            result = "(unnamed undeclared)";
        }
        else {
            result = common::get_tag_name(*declarationTag);
        }
    }
    else if (util::contains(result, "anonymous struct") ||
        util::contains(result, "anonymous union")) {
        result = common::get_tag_name(*type->getAsTagDecl());
    }

    // Remove trailing spaces after commas in template arguments
    clanguml::util::replace_all(result, ", ", ",");
    clanguml::util::replace_all(result, "> >", ">>");

    // Try to get rid of 'type-parameter-X-Y' ugliness
    if (result.find("type-parameter-") != std::string::npos) {
        util::if_not_null(
            common::dereference(type)->getAs<clang::TypedefType>(),
            [&result, &type](auto *p) {
                auto [unqualified_type, context] =
                    common::consume_type_context(type);
                result = p->getDecl()->getNameAsString();
                if (!context.empty()) {
                    std::vector<std::string> deduced_contexts;

                    for (const auto &c : context) {
                        deduced_contexts.push_back(c.to_string());
                    }

                    result = fmt::format(
                        "{} {}", result, fmt::join(deduced_contexts, " "));
                }
            });
    }

    return result;
}

std::string to_string(const clang::RecordType &type,
    const clang::ASTContext &ctx, bool try_canonical)
{
    return to_string(type.desugar(), ctx, try_canonical);
}

std::string to_string(
    const clang::TemplateArgument &arg, const clang::ASTContext *ctx)
{
    switch (arg.getKind()) {
    case clang::TemplateArgument::Expression:
        return to_string(arg.getAsExpr());
    case clang::TemplateArgument::Type:
        return to_string(arg.getAsType(), *ctx, false);
    case clang::TemplateArgument::Null:
        return "";
    case clang::TemplateArgument::NullPtr:
        return "nullptr";
    case clang::TemplateArgument::Integral:
        return std::to_string(arg.getAsIntegral().getExtValue());
    case clang::TemplateArgument::Template:
        return to_string(arg.getAsTemplate());
    case clang::TemplateArgument::TemplateExpansion:
        return to_string(arg.getAsTemplateOrTemplatePattern());
    default:
        return "";
    }
}

std::string to_string(const clang::TemplateName &templ)
{
    if (templ.getAsTemplateDecl() != nullptr) {
        return templ.getAsTemplateDecl()->getQualifiedNameAsString();
    }

    std::string result;
    const clang::LangOptions lang_options;
    llvm::raw_string_ostream ostream(result);
    templ.print(ostream, clang::PrintingPolicy(lang_options));

    return result;
}

std::string to_string(const clang::Expr *expr)
{
    const clang::LangOptions lang_options;
    std::string result;
    llvm::raw_string_ostream ostream(result);
    expr->printPretty(ostream, nullptr, clang::PrintingPolicy(lang_options));

    return result;
}

std::string to_string(const clang::ValueDecl *val)
{
    return val->getQualifiedNameAsString();
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

std::tuple<unsigned int, unsigned int, std::string>
extract_template_parameter_index(const std::string &type_parameter)
{
    assert(type_parameter.find("type-parameter-") == 0);

    auto type_parameter_and_suffix = util::split(type_parameter, " ");

    auto toks = util::split(
        type_parameter_and_suffix.front().substr(strlen("type-parameter-")),
        "-");

    std::string qualifier;

    if (type_parameter_and_suffix.size() > 1) {
        qualifier = type_parameter_and_suffix.at(1);
    }

    return {std::stoi(toks.at(0)), std::stoi(toks.at(1)), std::move(qualifier)};
}

void ensure_lambda_type_is_relative(
    const config::diagram &config, std::string &parameter_type)
{
#ifdef _MSC_VER
    auto root_name =
        fmt::format("{}", std::filesystem::current_path().root_name().string());
#else
    auto root_name = std::string{"/"};
#endif

    std::string lambda_prefix{fmt::format("(lambda at {}", root_name)};

    while (parameter_type.find(lambda_prefix) != std::string::npos) {
        auto lambda_begin = parameter_type.find(lambda_prefix);
        auto lambda_prefix_size = lambda_prefix.size();
#ifdef _MSC_VER
        // Skip the `\` or `/` after drive letter and semicolon
        lambda_prefix_size++;
#endif
        auto absolute_lambda_path_end =
            parameter_type.find(':', lambda_begin + lambda_prefix_size);
        auto absolute_lambda_path = parameter_type.substr(
            lambda_begin + lambda_prefix_size - 1,
            absolute_lambda_path_end - (lambda_begin + lambda_prefix_size - 1));

        auto relative_lambda_path = util::path_to_url(
            config.make_path_relative(absolute_lambda_path).string());

        parameter_type = fmt::format("{}(lambda at {}{}",
            parameter_type.substr(0, lambda_begin), relative_lambda_path,
            parameter_type.substr(absolute_lambda_path_end));
    }
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

template <> eid_t to_id(const std::string &full_name)
{
    return static_cast<eid_t>(
        static_cast<uint64_t>(std::hash<std::string>{}(full_name)));
}

eid_t to_id(const clang::QualType &type, const clang::ASTContext &ctx)
{
    return to_id(common::to_string(type, ctx));
}

template <> eid_t to_id(const clang::NamespaceDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> eid_t to_id(const clang::RecordDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> eid_t to_id(const clang::EnumDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> eid_t to_id(const clang::TagDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> eid_t to_id(const clang::CXXRecordDecl &declaration)
{
    return to_id(get_qualified_name(declaration));
}

template <> eid_t to_id(const clang::EnumType &t)
{
    return to_id(*t.getDecl());
}

template <> eid_t to_id(const std::filesystem::path &file)
{
    return to_id(file.lexically_normal().string());
}

template <> eid_t to_id(const clang::TemplateArgument &template_argument)
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
                ns_resolve(clanguml::util::trim_typename(type)));
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
            ns_resolve(clanguml::util::trim_typename(type)));
        type = "";
        for (auto &&param : nested_params)
            t.add_template_param(std::move(param));

        res.emplace_back(std::move(t));
    }

    return res;
}

bool is_type_parameter(const std::string &t)
{
    return t.find("type-parameter-") == 0;
}

bool is_qualifier(const std::string &q)
{
    return q == "&" || q == "&&" || q == "const&";
}

bool is_bracket(const std::string &b)
{
    return b == "(" || b == ")" || b == "[" || b == "]";
}

bool is_identifier_character(char c)
{
    return std::isalnum(c) != 0 || c == '_';
}

bool is_identifier(const std::string &t)
{
    return std::all_of(t.begin(), t.end(),
        [](const char c) { return is_identifier_character(c); });
}

bool is_keyword(const std::string &t)
{
    static std::vector<std::string> keywords{"alignas", "alignof", "asm",
        "auto", "bool", "break", "case", "catch", "char", "char16_t",
        "char32_t", "class", "concept", "const", "constexpr", "const_cast",
        "continue", "decltype", "default", "delete", "do", "double",
        "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false",
        "float", "for", "friend", "goto", "if", "inline", "int", "long",
        "mutable", "namespace", "new", "noexcept", "nullptr", "operator",
        "private", "protected", "public", "register", "reinterpret_cast",
        "return", "requires", "short", "signed", "sizeof", "static",
        "static_assert", "static_cast", "struct", "switch", "template", "this",
        "thread_local", "throw", "true", "try", "typedef", "typeid", "typename",
        "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t",
        "while"};

    return util::contains(keywords, t);
}

bool is_qualified_identifier(const std::string &t)
{
    return std::isalpha(t.at(0)) != 0 &&
        std::all_of(t.begin(), t.end(), [](const char c) {
            return is_identifier_character(c) || c == ':';
        });
}

bool is_type_token(const std::string &t)
{
    return is_type_parameter(t) ||
        (is_identifier(t) && !is_qualifier(t) && !is_bracket(t));
}

std::string format_condition_text(const std::string &condition_text)
{
    std::string result{condition_text};

    if (result.size() < 2)
        return {};

    std::vector<std::string> text_lines = util::split(result, "\n", true);

    // Trim each line
    for (auto &line : text_lines) {
        line = util::trim(line);
    }

    result = util::join(" ", text_lines);

    if (result.at(0) == '(' && result.back() == ')')
        return result.substr(1, result.size() - 2);

    return result;
}

std::string get_condition_text(clang::SourceManager &sm, clang::IfStmt *stmt)
{
    auto condition_range =
        clang::SourceRange(stmt->getLParenLoc(), stmt->getRParenLoc());

    return format_condition_text(get_source_text(condition_range, sm));
}

std::string get_condition_text(clang::SourceManager &sm, clang::WhileStmt *stmt)
{
    auto condition_range =
        clang::SourceRange(stmt->getLParenLoc(), stmt->getRParenLoc());

    return format_condition_text(get_source_text(condition_range, sm));
}

std::string get_condition_text(
    clang::SourceManager &sm, clang::CXXForRangeStmt *stmt)
{
    auto condition_range = stmt->getRangeStmt()->getSourceRange();

    return format_condition_text(get_source_text(condition_range, sm));
}

std::string get_condition_text(clang::SourceManager &sm, clang::ForStmt *stmt)
{
    auto condition_range =
        clang::SourceRange(stmt->getLParenLoc(), stmt->getRParenLoc());

    return format_condition_text(get_source_text(condition_range, sm));
}

std::string get_condition_text(clang::SourceManager &sm, clang::DoStmt *stmt)
{
    auto condition_range = stmt->getCond()->getSourceRange();

    return format_condition_text(get_source_text(condition_range, sm));
}

std::string get_condition_text(
    clang::SourceManager &sm, clang::ConditionalOperator *stmt)
{
    auto condition_range = stmt->getCond()->getSourceRange();

    return format_condition_text(get_source_text(condition_range, sm));
}

clang::QualType dereference(clang::QualType type)
{
    auto res = type;

    while (true) {
        if (res->isReferenceType())
            res = res.getNonReferenceType();
        else if (res->isPointerType())
            res = res->getPointeeType();
        else
            break;
    }

    return res;
}

std::pair<clang::QualType, std::deque<common::model::context>>
consume_type_context(clang::QualType type)
{
    std::deque<common::model::context> res;

    while (true) {
        bool try_again{false};
        common::model::context ctx;

        if (type.isConstQualified()) {
            ctx.is_const = true;
            try_again = true;
        }

        if (type.isVolatileQualified()) {
            ctx.is_volatile = true;
            try_again = true;
        }

        if (type->isPointerType() || type->isReferenceType()) {
            if (type.isConstQualified() || type.isVolatileQualified()) {
                ctx.is_ref_const = type.isConstQualified();
                ctx.is_ref_volatile = type.isVolatileQualified();

                try_again = true;
            }
        }

        if (type->isLValueReferenceType()) {
            ctx.pr = common::model::rpqualifier::kLValueReference;
            try_again = true;
        }
        else if (type->isRValueReferenceType()) {
            ctx.pr = common::model::rpqualifier::kRValueReference;
            try_again = true;
        }
        else if (type->isMemberFunctionPointerType() &&
            type->getPointeeType()->getAs<clang::FunctionProtoType>() !=
                nullptr) {
            const auto ref_qualifier =
                type->getPointeeType()                  // NOLINT
                    ->getAs<clang::FunctionProtoType>() // NOLINT
                    ->getRefQualifier();

            if (ref_qualifier == clang::RefQualifierKind::RQ_RValue) {
                ctx.pr = common::model::rpqualifier::kRValueReference;
                try_again = true;
            }
            else if (ref_qualifier == clang::RefQualifierKind::RQ_LValue) {
                ctx.pr = common::model::rpqualifier::kLValueReference;
                try_again = true;
            }
        }
        else if (type->isPointerType()) {
            ctx.pr = common::model::rpqualifier::kPointer;
            try_again = true;
        }

        if (try_again) {
            if (type->isPointerType()) {
                if (type->getPointeeType().isConstQualified())
                    ctx.is_const = true;
                if (type->getPointeeType().isVolatileQualified())
                    ctx.is_volatile = true;

                type = type->getPointeeType().getUnqualifiedType();
            }
            else if (type->isReferenceType()) {
                if (type.getNonReferenceType().isConstQualified())
                    ctx.is_const = true;
                if (type.getNonReferenceType().isVolatileQualified())
                    ctx.is_volatile = true;

                type = type.getNonReferenceType().getUnqualifiedType();
            }
            else if (type.isConstQualified() || type.isVolatileQualified()) {
                ctx.is_const = type.isConstQualified();
                ctx.is_volatile = type.isVolatileQualified();
                type = type.getUnqualifiedType();
            }

            res.push_front(ctx);

            if (type->isMemberFunctionPointerType())
                return std::make_pair(type, res);
        }
        else
            return std::make_pair(type, res);
    }
}

std::vector<std::string> tokenize_unexposed_template_parameter(
    const std::string &t)
{
    std::vector<std::string> result;

    auto spaced_out = util::split(t, " ");

    for (const auto &word : spaced_out) {
        if (is_qualified_identifier(word)) {
            if (word != "class" && word != "templated" && word != "struct")
                result.emplace_back(word);
            continue;
        }

        std::string tok;

        for (const char c : word) {
            if (c == '(' || c == ')' || c == '[' || c == ']' || c == '<' ||
                c == '>') {
                if (!tok.empty())
                    result.emplace_back(tok);
                result.emplace_back(std::string{c});
                tok.clear();
            }
            else if (c == ':') {
                if (!tok.empty() && tok != ":") {
                    result.emplace_back(tok);
                    tok = ":";
                }
                else if (tok == ":") {
                    result.emplace_back("::");
                    tok = "";
                }
                else {
                    tok += ':';
                }
            }
            else if (c == ',') {
                if (!tok.empty()) {
                    result.emplace_back(tok);
                }
                result.emplace_back(",");
                tok.clear();
            }
            else if (c == '*') {
                if (!tok.empty()) {
                    result.emplace_back(tok);
                }
                result.emplace_back("*");
                tok.clear();
            }
            else if (c == '.') {
                // This can only be the case if we have a variadic template,
                // right?
                if (tok == "..") {
                    result.emplace_back("...");
                    tok.clear();
                }
                else if (tok == ".") {
                    tok = "..";
                }
                else if (!tok.empty()) {
                    result.emplace_back(tok);
                    tok = ".";
                }
            }
            else {
                tok += c;
            }
        }

        tok = util::trim(tok);

        if (!tok.empty()) {
            if (tok != "class" && tok != "typename" && word != "struct")
                result.emplace_back(tok);
            tok.clear();
        }
    }

    return result;
}

bool parse_source_location(const std::string &location_str, std::string &file,
    unsigned &line, unsigned &column)
{
    auto tokens = util::split(location_str, ":");

    if (tokens.size() < 3)
        return false;

    if (tokens.size() == 4) {
        // Handle Windows paths
        decltype(tokens) tmp_tokens{};
        tmp_tokens.emplace_back(
            fmt::format("{}:{}", tokens.at(0), tokens.at(1)));
        tmp_tokens.emplace_back(tokens.at(2));
        tmp_tokens.emplace_back(tokens.at(3));

        tokens = std::move(tmp_tokens);
    }

    file = tokens.at(0);
    try {
        line = std::stoi(tokens.at(1));
    }
    catch (std::invalid_argument &e) {
        return false;
    }

    try {
        column = std::stoi(tokens.at(2));
    }
    catch (std::invalid_argument &e) {
        column = 0;
    }

    return true;
}

clang::RawComment *get_expression_raw_comment(const clang::SourceManager &sm,
    const clang::ASTContext &context, const clang::Stmt *stmt)
{
    // First get the first line of the expression
    auto expr_begin = stmt->getSourceRange().getBegin();
    const auto expr_begin_line = sm.getSpellingLineNumber(expr_begin);

    if (!context.Comments.empty() &&
        context.Comments.getCommentsInFile(sm.getFileID(expr_begin)) != nullptr)
        for (const auto [offset, raw_comment] :
            *context.Comments.getCommentsInFile(sm.getFileID(expr_begin))) {
            const auto comment_end_line = sm.getSpellingLineNumber(
                raw_comment->getSourceRange().getEnd());

            if (expr_begin_line == comment_end_line ||
                expr_begin_line == comment_end_line + 1)
                return raw_comment;
        }

    return {};
}

bool is_coroutine(const clang::FunctionDecl &decl)
{
    const auto *body = decl.getBody();
    return clang::isa_and_nonnull<clang::CoroutineBodyStmt>(body);
}

bool is_struct(const clang::NamedDecl *decl)
{
    if (decl == nullptr)
        return false;

    if (const auto *record = clang::dyn_cast<clang::CXXRecordDecl>(decl);
        record) {
        return record->isStruct();
    }

    if (const auto *tag = clang::dyn_cast<clang::TagDecl>(decl); tag) {
        return tag->isStruct();
    }

    return false;
}

bool has_attr(const clang::FunctionDecl *decl, clang::attr::Kind function_attr)
{
    return std::any_of(decl->attrs().begin(), decl->attrs().end(),
        [function_attr](
            auto &&attr) { return attr->getKind() == function_attr; });
}

std::optional<size_t> get_array_size(const clang::ArrayType &type)
{
    if (const auto *constant_array =
            clang::dyn_cast<clang::ConstantArrayType>(&type);
        constant_array != nullptr) {
        return {constant_array->getSize().getZExtValue()};
    }

    return {};
}
} // namespace clanguml::common
