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

namespace clanguml::common {

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

template <> id_t to_id(const clang::TemplateSpecializationType &t)
{
    return t.getTemplateName().getAsTemplateDecl()->getID();
}

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
