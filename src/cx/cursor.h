/**
 * src/cx/cursor.h
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

#include "cx/type.h"

#include <list>
#include <string>

namespace clanguml {
namespace cx {

class cursor {
public:
    cursor();

    cursor(CXCursor &&c);

    cursor(const CXCursor &c);

    cursor(const cursor &c);

    ~cursor() = default;

    bool operator==(const cursor &b) const;

    cx::type type() const;
    std::string display_name() const;

    std::string spelling() const;

    bool is_void() const;

    /**
     * @brief Return fully qualified cursor spelling
     *
     * This method generates a fully qualified name for the cursor by
     * traversing the namespaces upwards.
     *
     * TODO: Add caching of this value.
     *
     * @return Fully qualified cursor spelling
     */
    std::string fully_qualified() const;

    cursor referenced() const;

    cursor semantic_parent() const;

    cursor lexical_parent() const;

    CXCursorKind kind() const;
    std::string kind_spelling() const;

    cursor definition() const;
    bool is_definition() const;
    bool is_declaration() const;
    bool is_forward_declaration() const;

    bool is_invalid_declaration() const;
    CXSourceLocation location() const;

    bool is_reference() const;
    bool is_expression() const;
    bool is_statement() const;
    bool is_namespace() const;
    bool is_attribute() const;
    bool has_attrs() const;
    bool is_invalid() const;
    bool is_translation_unit() const;
    bool is_preprocessing() const;
    bool is_method_virtual() const;

    bool is_static() const;

    bool is_method_static() const;
    bool is_method_const() const;
    bool is_method_pure_virtual() const;

    bool is_method_defaulted() const;

    bool is_method_parameter() const;
    CXVisibilityKind visibitity() const;

    CXAvailabilityKind availability() const;

    CX_CXXAccessSpecifier cxxaccess_specifier() const;

    cx::type underlying_type() const;

    int template_argument_count() const;

    CXTemplateArgumentKind template_argument_kind(unsigned i) const;

    cx::type template_argument_type(unsigned i) const;

    long long template_argument_value(unsigned i) const;

    cursor specialized_cursor_template() const;

    CXTranslationUnit translation_unit() const;

    bool is_template_parameter_variadic() const;

    std::string usr() const;
    CXSourceRange extent() const;
    std::vector<std::string> tokenize() const;

    std::string default_value() const;

    std::vector<std::string> tokenize_template_parameters() const;

    const CXCursor &get() const;

private:
    CXCursor m_cursor;
};
}
}

template <> struct fmt::formatter<clanguml::cx::cursor> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const clanguml::cx::cursor &c, FormatContext &ctx)
    {
        return fmt::format_to(ctx.out(),
            "(cx::cursor spelling={}, display_name={}, kind={}, "
            "is_expression={}, template_argument_count={})",
            c.spelling(), c.display_name(), c.kind_spelling(),
            c.is_expression(), c.template_argument_count()

        );
    }
};
