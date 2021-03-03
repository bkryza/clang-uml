/**
 * src/cx/type.h
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include "uml/class_diagram_model.h"
#include "util/util.h"

namespace clanguml {
namespace cx {

namespace detail {
std::string to_string(CXString &&cxs)
{
    std::string r{clang_getCString(cxs)};
    clang_disposeString(cxs);
    return r;
}
}

using detail::to_string;

class type {
public:
    type(CXType &&t)
        : m_type{std::move(t)}
    {
    }
    ~type() = default;

    std::string spelling() const
    {
        return to_string(clang_getTypeSpelling(m_type));
    }

    bool operator==(const type &b) const
    {
        return clang_equalTypes(m_type, b.get());
    }

    type canonical() const { return {clang_getCanonicalType(m_type)}; }

    bool is_const_qualified() const
    {
        return clang_isConstQualifiedType(m_type);
    }

    bool is_volatile_qualified() const
    {
        return clang_isVolatileQualifiedType(m_type);
    }

    bool is_restricted_qualified() const
    {
        return clang_isRestrictQualifiedType(m_type);
    }

    std::string typedef_name() const
    {
        return to_string(clang_getTypedefName(m_type));
    }

    type pointee_type() const { return {clang_getPointeeType(m_type)}; }

    /*
     *cursor type_declaration() const
     *{
     *    return {clang_getTypeDeclaration(m_type)};
     *}
     */

    /*
     *std::string type_kind_spelling() const
     *{
     *    return clang_getTypeKindSpelling(kind());
     *}
     */

    CXTypeKind kind() const { return m_type.kind; }

    std::string kind_spelling()
    {
        return to_string(clang_getTypeKindSpelling(m_type.kind));
    }

    CXCallingConv calling_convention() const
    {
        return clang_getFunctionTypeCallingConv(m_type);
    }

    type result_type() const { return clang_getResultType(m_type); }

    int exception_specification_type() const
    {
        return clang_getExceptionSpecificationType(m_type);
    }

    int argument_type_count() const { return clang_getNumArgTypes(m_type); }

    type argument_type(int i) const { return {clang_getArgType(m_type, i)}; }

    bool is_function_variadic() const
    {
        return clang_isFunctionTypeVariadic(m_type);
    }

    bool is_pod() const { return clang_isPODType(m_type); }

    bool is_pointer() const { return kind() == CXType_Pointer; }

    bool is_record() const { return kind() == CXType_Record; }

    /**
     * @brief Return final referenced type.
     *
     * This method allows to extract a final type in case a type consists of a
     * single or multiple pointers or references.
     *
     * @return Referenced type.
     */
    type referenced() const
    {
        auto t = *this;
        while (t.is_pointer() || t.is_reference()) {
            t = t.pointee_type();
        }

        return t;
    }

    bool is_reference() const
    {
        return (kind() == CXType_LValueReference) ||
            (kind() == CXType_RValueReference);
    }

    bool is_array() const { return clang_getArraySize(m_type) > -1; }

    type array_type() const { return {clang_getArrayElementType(m_type)}; }

    bool is_relationship() const
    {
        return is_pointer() || is_record() || is_reference() || !is_pod() ||
            is_array() || (spelling().find("std::array") == 0);
    }

    type element_type() const { return clang_getElementType(m_type); }

    long long element_count() const { return clang_getNumElements(m_type); }

    type array_element_type() const
    {
        return clang_getArrayElementType(m_type);
    }

    type named_type() const { return clang_Type_getNamedType(m_type); }

    CXTypeNullabilityKind nullability() const
    {
        return clang_Type_getNullability(m_type);
    }

    type class_type() const { return clang_Type_getClassType(m_type); }

    long long size_of() const { return clang_Type_getSizeOf(m_type); }

    type modified_type() const { return clang_Type_getModifiedType(m_type); }

    type value_type() const { return clang_Type_getValueType(m_type); }

    bool is_template() const { return template_arguments_count() > 0; }

    int template_arguments_count() const
    {
        return clang_Type_getNumTemplateArguments(m_type);
    }

    type template_argument_type(int i) const
    {
        return clang_Type_getTemplateArgumentAsType(m_type, i);
    }

    const CXType &get() const { return m_type; }

    CXRefQualifierKind cxxref_qualifier() const
    {
        return clang_Type_getCXXRefQualifier(m_type);
    }

    std::string unqualified() const
    {
        auto toks = clanguml::util::split(spelling(), " ");
        const std::vector<std::string> qualifiers = {
            "static", "const", "volatile", "register", "mutable"};

        toks.erase(toks.begin(),
            std::find_if(
                toks.begin(), toks.end(), [&qualifiers](const auto &t) {
                    return std::count(
                               qualifiers.begin(), qualifiers.end(), t) == 0;
                }));

        return fmt::format("{}", fmt::join(toks, " "));
    }

private:
    CXType m_type;
};
}
}
