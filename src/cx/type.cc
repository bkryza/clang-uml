/**
 * src/cx/type.cc
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
#include "type.h"
#include "cursor.h"

namespace clanguml {
namespace cx {

type::type(CXType &&t)
    : m_type{std::move(t)}
{
}

std::string type::spelling() const
{
    return to_string(clang_getTypeSpelling(m_type));
}

bool type::operator==(const type &b) const
{
    return clang_equalTypes(m_type, b.get());
}

type type::canonical() const { return {clang_getCanonicalType(m_type)}; }

bool type::is_unexposed() const { return kind() == CXType_Unexposed; }

bool type::is_const_qualified() const
{
    return clang_isConstQualifiedType(m_type);
}

bool type::is_volatile_qualified() const
{
    return clang_isVolatileQualifiedType(m_type);
}

bool type::is_restricted_qualified() const
{
    return clang_isRestrictQualifiedType(m_type);
}

bool type::is_invalid() const { return kind() == CXType_Invalid; }

std::string type::typedef_name() const
{
    return to_string(clang_getTypedefName(m_type));
}

type type::pointee_type() const { return {clang_getPointeeType(m_type)}; }

CXTypeKind type::kind() const { return m_type.kind; }

std::string type::kind_spelling() const
{
    return to_string(clang_getTypeKindSpelling(m_type.kind));
}

CXCallingConv type::calling_convention() const
{
    return clang_getFunctionTypeCallingConv(m_type);
}

type type::result_type() const { return clang_getResultType(m_type); }

int type::exception_specification_type() const
{
    return clang_getExceptionSpecificationType(m_type);
}

int type::argument_type_count() const { return clang_getNumArgTypes(m_type); }

type type::argument_type(int i) const { return {clang_getArgType(m_type, i)}; }

bool type::is_function_variadic() const
{
    return clang_isFunctionTypeVariadic(m_type);
}

bool type::is_pod() const { return clang_isPODType(m_type); }

bool type::is_pointer() const { return kind() == CXType_Pointer; }

bool type::is_record() const { return kind() == CXType_Record; }

type type::referenced() const
{
    auto t = *this;
    while (t.is_pointer() || t.is_reference()) {
        t = t.pointee_type();
    }

    return t;
}

bool type::is_reference() const
{
    return (kind() == CXType_LValueReference) ||
        (kind() == CXType_RValueReference);
}

bool type::is_array() const { return clang_getArraySize(m_type) > -1; }

type type::array_type() const { return {clang_getArrayElementType(m_type)}; }

bool type::is_relationship() const
{
    return is_pointer() || is_record() || is_reference() || !is_pod() ||
        is_array() || is_template() ||
        (spelling().find("std::array") ==
            0 /* There must be a better way... */);
}

type type::element_type() const { return clang_getElementType(m_type); }

long long type::element_count() const { return clang_getNumElements(m_type); }

type type::array_element_type() const
{
    return clang_getArrayElementType(m_type);
}

type type::named_type() const { return clang_Type_getNamedType(m_type); }

CXTypeNullabilityKind type::nullability() const
{
    return clang_Type_getNullability(m_type);
}

type type::class_type() const { return clang_Type_getClassType(m_type); }

long long type::size_of() const { return clang_Type_getSizeOf(m_type); }

type type::modified_type() const { return clang_Type_getModifiedType(m_type); }

type type::value_type() const { return clang_Type_getValueType(m_type); }

bool type::is_template() const { return template_arguments_count() > 0; }

bool type::is_template_parameter() const
{
    return canonical().spelling().find("type-parameter-") == 0;
}

int type::template_arguments_count() const
{
    return clang_Type_getNumTemplateArguments(m_type);
}

type type::template_argument_type(int i) const
{
    return clang_Type_getTemplateArgumentAsType(m_type, i);
}

const CXType &type::get() const { return m_type; }

CXRefQualifierKind type::cxxref_qualifier() const
{
    return clang_Type_getCXXRefQualifier(m_type);
}

std::string type::unqualified() const
{
    return clanguml::util::unqualify(spelling());
}

cursor type::type_declaration() const
{
    return {clang_getTypeDeclaration(m_type)};
}

std::string type::instantiation_template() const
{
    assert(is_template_instantiation());

    auto s = spelling();
    auto it = s.find('<');
    auto template_base_name = s.substr(0, it);

    auto cur = type_declaration();

    return cur.fully_qualified();
}

bool type::is_template_instantiation() const
{
    auto s = spelling();
    auto it = s.find('<');
    return it != std::string::npos &&
        referenced().type_declaration().kind() != CXCursor_ClassTemplate;
}
}
}
