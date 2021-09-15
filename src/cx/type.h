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

#include "cx/util.h"
#include "uml/class_diagram_model.h"
#include "util/util.h"

namespace clanguml {
namespace cx {

using util::to_string;

class cursor;

class type {
public:
    type(CXType &&t);

    ~type() = default;

    std::string spelling() const;

    bool operator==(const type &b) const;

    type canonical() const;
    bool is_unexposed() const;
    bool is_const_qualified() const;

    bool is_volatile_qualified() const;

    bool is_restricted_qualified() const;

    bool is_invalid() const;
    std::string typedef_name() const;

    type pointee_type() const;
    cursor type_declaration() const;

    CXTypeKind kind() const;
    std::string kind_spelling() const;

    CXCallingConv calling_convention() const;

    type result_type() const;
    int exception_specification_type() const;

    int argument_type_count() const;
    type argument_type(int i) const;
    bool is_function_variadic() const;

    bool is_pod() const;
    bool is_pointer() const;
    bool is_record() const;
    /**
     * @brief Return final referenced type.
     *
     * This method allows to extract a final type in case a type consists of a
     * single or multiple pointers or references.
     *
     * @return Referenced type.
     */
    type referenced() const;

    bool is_reference() const;

    bool is_array() const;
    type array_type() const;
    bool is_relationship() const;

    type element_type() const;
    long long element_count() const;
    type array_element_type() const;

    type named_type() const;
    CXTypeNullabilityKind nullability() const;

    type class_type() const;
    long long size_of() const;
    type modified_type() const;
    type value_type() const;
    bool is_template() const;
    bool is_template_parameter() const;

    int template_arguments_count() const;

    type template_argument_type(int i) const;

    const CXType &get() const;
    CXRefQualifierKind cxxref_qualifier() const;

    bool is_template_instantiation() const;

    std::string instantiation_template() const;

    /**
     * @brief Remove all qualifiers from field declaration.
     *
     * @return Unqualified identifier.
     */
    std::string unqualified() const;

private:
    CXType m_type;
};
}
}

template <> struct fmt::formatter<clanguml::cx::type> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const clanguml::cx::type &t, FormatContext &ctx)
    {
        return fmt::format_to(ctx.out(),
            "(cx::type spelling={}, kind={}, pointee={}, "
            "is_pod={}, canonical={}, is_relationship={})",
            t.spelling(), t.kind_spelling(), t.pointee_type().spelling(),
            t.is_pod(), t.canonical().spelling(), t.is_relationship());
    }
};
