/**
 * @file src/class_diagram/visitor/template_builder.cc
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

#include "template_builder.h"
#include "common/clang_utils.h"
#include <clang/Lex/Lexer.h>

namespace clanguml::common::visitor {
namespace detail {

std::string map_type_parameter_to_template_parameter(
    const clang::ClassTemplateSpecializationDecl *decl, const std::string &tp)
{
    const auto [depth0, index0, qualifier0] =
        common::extract_template_parameter_index(tp);

    for (auto i = 0U; i < decl->getDescribedTemplateParams()->size(); i++) {
        const auto *param = decl->getDescribedTemplateParams()->getParam(i);

        if (i == index0) {
            return param->getNameAsString();
        }
    }

    return tp;
}

std::string map_type_parameter_to_template_parameter(
    const clang::TypeAliasTemplateDecl *decl, const std::string &tp)
{
    const auto [depth0, index0, qualifier0] =
        common::extract_template_parameter_index(tp);

    for (auto i = 0U; i < decl->getTemplateParameters()->size(); i++) {
        const auto *param = decl->getTemplateParameters()->getParam(i);

        if (i == index0) {
            return param->getNameAsString();
        }
    }

    return tp;
}

} // namespace detail

std::string map_type_parameter_to_template_parameter_name(
    const clang::Decl *decl, const std::string &type_parameter)
{
    if (type_parameter.find("type-parameter-") != 0)
        return type_parameter;

    if (const auto *template_decl =
            llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl);
        template_decl != nullptr) {
        return detail::map_type_parameter_to_template_parameter(
            template_decl, type_parameter);
    }

    if (const auto *alias_decl =
            llvm::dyn_cast<clang::TypeAliasTemplateDecl>(decl);
        alias_decl != nullptr) {
        return detail::map_type_parameter_to_template_parameter(
            alias_decl, type_parameter);
    }

    // Fallback
    return type_parameter;
}

} // namespace clanguml::common::visitor
